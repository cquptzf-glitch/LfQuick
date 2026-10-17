#pragma once

#include <LfQuick/Io/LfQuickIoGlobal.h>

#include <QByteArray>
#include <QObject>
#include <QPointer>
#include <QStringList>
#include <QVariantMap>
#include <QtQmlIntegration/qqmlintegration.h>

class QProcess;

namespace LfQuick {

class DataStreamParser;

class LFQUICKIO_EXPORT ProcessRunner final : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LfProcess)
    Q_PROPERTY(QString program READ program WRITE setProgram NOTIFY programChanged FINAL)
    Q_PROPERTY(QStringList arguments READ arguments WRITE setArguments NOTIFY argumentsChanged FINAL)
    Q_PROPERTY(QString workingDirectory READ workingDirectory WRITE setWorkingDirectory NOTIFY workingDirectoryChanged FINAL)
    Q_PROPERTY(QVariantMap environment READ environment WRITE setEnvironment NOTIFY environmentChanged FINAL)
    Q_PROPERTY(State state READ state NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool running READ running NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool mergeChannels READ mergeChannels WRITE setMergeChannels NOTIFY mergeChannelsChanged FINAL)
    Q_PROPERTY(qint64 maxCapturedBytes READ maxCapturedBytes WRITE setMaxCapturedBytes NOTIFY maxCapturedBytesChanged FINAL)
    Q_PROPERTY(QByteArray standardOutput READ standardOutput NOTIFY outputChanged FINAL)
    Q_PROPERTY(QByteArray standardError READ standardError NOTIFY outputChanged FINAL)
    Q_PROPERTY(QString standardOutputText READ standardOutputText NOTIFY outputChanged FINAL)
    Q_PROPERTY(QString standardErrorText READ standardErrorText NOTIFY outputChanged FINAL)
    Q_PROPERTY(int exitCode READ exitCode NOTIFY exitStatusChanged FINAL)
    Q_PROPERTY(ExitStatus exitStatus READ exitStatus NOTIFY exitStatusChanged FINAL)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged FINAL)
    Q_PROPERTY(DataStreamParser* stdoutParser READ stdoutParser WRITE setStdoutParser NOTIFY stdoutParserChanged FINAL)
    Q_PROPERTY(DataStreamParser* stderrParser READ stderrParser WRITE setStderrParser NOTIFY stderrParserChanged FINAL)

public:
    enum class State {
        NotRunning,
        Starting,
        Running
    };
    Q_ENUM(State)

    enum class ExitStatus {
        Unknown,
        NormalExit,
        CrashExit
    };
    Q_ENUM(ExitStatus)

    explicit ProcessRunner(QObject *parent = nullptr);
    ~ProcessRunner() override;

    QString program() const { return m_program; }
    void setProgram(const QString &program);
    QStringList arguments() const { return m_arguments; }
    void setArguments(const QStringList &arguments);
    QString workingDirectory() const { return m_workingDirectory; }
    void setWorkingDirectory(const QString &workingDirectory);
    QVariantMap environment() const { return m_environment; }
    void setEnvironment(const QVariantMap &environment);

    State state() const noexcept { return m_state; }
    bool running() const noexcept { return m_state != State::NotRunning; }

    bool mergeChannels() const noexcept { return m_mergeChannels; }
    void setMergeChannels(bool mergeChannels);
    qint64 maxCapturedBytes() const noexcept { return m_maxCapturedBytes; }
    void setMaxCapturedBytes(qint64 bytes);

    QByteArray standardOutput() const { return m_stdout; }
    QByteArray standardError() const { return m_stderr; }
    QString standardOutputText() const { return QString::fromUtf8(m_stdout); }
    QString standardErrorText() const { return QString::fromUtf8(m_stderr); }

    int exitCode() const noexcept { return m_exitCode; }
    ExitStatus exitStatus() const noexcept { return m_exitStatus; }
    QString errorString() const { return m_errorString; }

    DataStreamParser *stdoutParser() const { return m_stdoutParser; }
    void setStdoutParser(DataStreamParser *parser);
    DataStreamParser *stderrParser() const { return m_stderrParser; }
    void setStderrParser(DataStreamParser *parser);

    Q_INVOKABLE bool start();
    Q_INVOKABLE void terminate();
    Q_INVOKABLE void kill();
    Q_INVOKABLE qint64 write(const QByteArray &data);
    Q_INVOKABLE void closeWriteChannel();
    Q_INVOKABLE void clearOutput();

signals:
    void programChanged();
    void argumentsChanged();
    void workingDirectoryChanged();
    void environmentChanged();
    void stateChanged();
    void mergeChannelsChanged();
    void maxCapturedBytesChanged();
    void outputChanged();
    void exitStatusChanged();
    void errorStringChanged();
    void stdoutParserChanged();
    void stderrParserChanged();
    void started();
    void finished(int exitCode, ExitStatus status);
    void standardOutputReceived(const QByteArray &data);
    void standardErrorReceived(const QByteArray &data);
    void outputTruncated(const QString &channel, qint64 droppedBytes);

private:
    void handleStdout();
    void handleStderr();
    void appendCaptured(QByteArray &target, const QByteArray &data, const QString &channel);
    void setState(State state);
    void setErrorString(const QString &error);
    void finishParsers();

    QProcess *m_process = nullptr;
    QString m_program;
    QStringList m_arguments;
    QString m_workingDirectory;
    QVariantMap m_environment;
    State m_state = State::NotRunning;
    bool m_mergeChannels = false;
    qint64 m_maxCapturedBytes = 4 * 1024 * 1024;
    QByteArray m_stdout;
    QByteArray m_stderr;
    int m_exitCode = -1;
    ExitStatus m_exitStatus = ExitStatus::Unknown;
    QString m_errorString;
    QPointer<DataStreamParser> m_stdoutParser;
    QPointer<DataStreamParser> m_stderrParser;
    QByteArray m_stdoutParserBuffer;
    QByteArray m_stderrParserBuffer;
};

} // namespace LfQuick
