#include <LfQuick/Io/ProcessRunner.h>

#include <QDir>
#include <QProcess>
#include <QProcessEnvironment>

#include <algorithm>

namespace LfQuick {

ProcessRunner::ProcessRunner(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
{
    connect(m_process, &QProcess::started, this, [this] {
        setState(State::Running);
        emit started();
    });
    connect(m_process, &QProcess::stateChanged, this, [this](QProcess::ProcessState state) {
        switch (state) {
        case QProcess::NotRunning: setState(State::NotRunning); break;
        case QProcess::Starting: setState(State::Starting); break;
        case QProcess::Running: setState(State::Running); break;
        }
    });
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &ProcessRunner::handleStdout);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &ProcessRunner::handleStderr);
    connect(m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        setErrorString(m_process->errorString());
        if (error == QProcess::FailedToStart) {
            setState(State::NotRunning);
            completeRun(-1, ExitStatus::Unknown);
        }
    });
    connect(m_process,
            qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this,
            [this](int exitCode, QProcess::ExitStatus status) {
        completeRun(exitCode,
                    status == QProcess::NormalExit
                        ? ExitStatus::NormalExit
                        : ExitStatus::CrashExit);
    });
}

ProcessRunner::~ProcessRunner()
{
    QObject::disconnect(m_stdoutParserDestroyedConnection);
    QObject::disconnect(m_stderrParserDestroyedConnection);
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(100);
    }
}

void ProcessRunner::setProgram(const QString &program)
{
    if (m_program == program)
        return;
    m_program = program;
    emit programChanged();
}

void ProcessRunner::setArguments(const QStringList &arguments)
{
    if (m_arguments == arguments)
        return;
    m_arguments = arguments;
    emit argumentsChanged();
}

void ProcessRunner::setWorkingDirectory(const QString &workingDirectory)
{
    const QString normalized = workingDirectory.trimmed().isEmpty()
        ? QString{}
        : QDir::cleanPath(workingDirectory);
    if (m_workingDirectory == normalized)
        return;
    m_workingDirectory = normalized;
    emit workingDirectoryChanged();
}

void ProcessRunner::setEnvironment(const QVariantMap &environment)
{
    if (m_environment == environment)
        return;
    m_environment = environment;
    emit environmentChanged();
}

void ProcessRunner::setMergeChannels(bool mergeChannels)
{
    if (m_mergeChannels == mergeChannels)
        return;
    if (running()) {
        setErrorString(QStringLiteral("Channel mode cannot change while process is running"));
        return;
    }
    m_mergeChannels = mergeChannels;
    emit mergeChannelsChanged();
}

void ProcessRunner::setMaxCapturedBytes(qint64 bytes)
{
    bytes = std::max<qint64>(0, bytes);
    if (m_maxCapturedBytes == bytes)
        return;
    m_maxCapturedBytes = bytes;

    const auto trim = [this](QByteArray &data, const QString &channel) {
        if (m_maxCapturedBytes <= 0 || data.size() <= m_maxCapturedBytes)
            return;
        const qint64 dropped = data.size() - m_maxCapturedBytes;
        data.remove(0, static_cast<qsizetype>(dropped));
        emit outputTruncated(channel, dropped);
    };
    trim(m_stdout, QStringLiteral("stdout"));
    trim(m_stderr, QStringLiteral("stderr"));
    emit maxCapturedBytesChanged();
    emit outputChanged();
}

void ProcessRunner::setStdoutParser(DataStreamParser *parser)
{
    if (m_stdoutParser == parser)
        return;

    QObject::disconnect(m_stdoutParserDestroyedConnection);
    m_stdoutParserDestroyedConnection = {};
    if (m_stdoutParser)
        m_stdoutParser->streamEnded(m_stdoutParserBuffer);
    m_stdoutParserBuffer.clear();
    m_stdoutParser = parser;
    if (m_stdoutParser) {
        m_stdoutParserDestroyedConnection = connect(
            m_stdoutParser, &QObject::destroyed, this, [this] {
                m_stdoutParser = nullptr;
                m_stdoutParserBuffer.clear();
                m_stdoutParserDestroyedConnection = {};
                emit stdoutParserChanged();
            });
    }
    emit stdoutParserChanged();
}

void ProcessRunner::setStderrParser(DataStreamParser *parser)
{
    if (m_stderrParser == parser)
        return;

    QObject::disconnect(m_stderrParserDestroyedConnection);
    m_stderrParserDestroyedConnection = {};
    if (m_stderrParser)
        m_stderrParser->streamEnded(m_stderrParserBuffer);
    m_stderrParserBuffer.clear();
    m_stderrParser = parser;
    if (m_stderrParser) {
        m_stderrParserDestroyedConnection = connect(
            m_stderrParser, &QObject::destroyed, this, [this] {
                m_stderrParser = nullptr;
                m_stderrParserBuffer.clear();
                m_stderrParserDestroyedConnection = {};
                emit stderrParserChanged();
            });
    }
    emit stderrParserChanged();
}

bool ProcessRunner::start()
{
    if (!m_process || running())
        return false;
    if (m_program.trimmed().isEmpty()) {
        setErrorString(QStringLiteral("Process program is empty"));
        return false;
    }

    clearOutput();
    setErrorString({});
    m_exitCode = -1;
    m_exitStatus = ExitStatus::Unknown;
    m_runCompletionEmitted = false;
    emit exitStatusChanged();

    m_process->setWorkingDirectory(m_workingDirectory);
    m_process->setProcessChannelMode(m_mergeChannels
                                         ? QProcess::MergedChannels
                                         : QProcess::SeparateChannels);

    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    for (auto it = m_environment.cbegin(); it != m_environment.cend(); ++it) {
        if (!it.value().isValid() || it.value().isNull())
            environment.remove(it.key());
        else
            environment.insert(it.key(), it.value().toString());
    }
    m_process->setProcessEnvironment(environment);

    // Enter Starting before QProcess::start(). FailedToStart may be reported
    // synchronously; setting the state afterwards would incorrectly overwrite
    // the NotRunning transition produced by the error path.
    setState(State::Starting);
    m_process->start(m_program, m_arguments);
    return true;
}

void ProcessRunner::terminate()
{
    if (m_process && running())
        m_process->terminate();
}

void ProcessRunner::kill()
{
    if (m_process && running())
        m_process->kill();
}

qint64 ProcessRunner::write(const QByteArray &data)
{
    if (!m_process || m_process->state() != QProcess::Running)
        return -1;
    return m_process->write(data);
}

void ProcessRunner::closeWriteChannel()
{
    if (m_process)
        m_process->closeWriteChannel();
}

void ProcessRunner::clearOutput()
{
    const bool changed = !m_stdout.isEmpty() || !m_stderr.isEmpty();
    m_stdout.clear();
    m_stderr.clear();
    if (changed)
        emit outputChanged();
}

void ProcessRunner::handleStdout()
{
    if (!m_process)
        return;
    const QByteArray data = m_process->readAllStandardOutput();
    if (data.isEmpty())
        return;

    appendCaptured(m_stdout, data, QStringLiteral("stdout"));
    emit standardOutputReceived(data);
    emit outputChanged();
    if (m_stdoutParser) {
        QByteArray incoming = data;
        m_stdoutParser->parseBytes(incoming, m_stdoutParserBuffer);
    }
}

void ProcessRunner::handleStderr()
{
    if (!m_process || m_mergeChannels)
        return;
    const QByteArray data = m_process->readAllStandardError();
    if (data.isEmpty())
        return;

    appendCaptured(m_stderr, data, QStringLiteral("stderr"));
    emit standardErrorReceived(data);
    emit outputChanged();
    if (m_stderrParser) {
        QByteArray incoming = data;
        m_stderrParser->parseBytes(incoming, m_stderrParserBuffer);
    }
}

void ProcessRunner::appendCaptured(QByteArray &target,
                                   const QByteArray &data,
                                   const QString &channel)
{
    target.append(data);
    if (m_maxCapturedBytes <= 0 || target.size() <= m_maxCapturedBytes)
        return;

    const qint64 dropped = target.size() - m_maxCapturedBytes;
    target.remove(0, static_cast<qsizetype>(dropped));
    emit outputTruncated(channel, dropped);
}

void ProcessRunner::setState(State state)
{
    if (m_state == state)
        return;
    m_state = state;
    emit stateChanged();
}

void ProcessRunner::setErrorString(const QString &error)
{
    if (m_errorString == error)
        return;
    m_errorString = error;
    emit errorStringChanged();
}

void ProcessRunner::finishParsers()
{
    if (m_stdoutParser)
        m_stdoutParser->streamEnded(m_stdoutParserBuffer);
    m_stdoutParserBuffer.clear();

    if (m_stderrParser)
        m_stderrParser->streamEnded(m_stderrParserBuffer);
    m_stderrParserBuffer.clear();
}

void ProcessRunner::completeRun(int exitCode, ExitStatus status)
{
    if (m_runCompletionEmitted)
        return;
    m_runCompletionEmitted = true;

    handleStdout();
    handleStderr();
    finishParsers();
    m_exitCode = exitCode;
    m_exitStatus = status;
    setState(State::NotRunning);
    emit exitStatusChanged();
    emit finished(m_exitCode, m_exitStatus);
}

} // namespace LfQuick
