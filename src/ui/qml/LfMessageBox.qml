import QtQuick

QtObject {
    id: root

    property OverlayCoordinator coordinator: null

    function info(title, message, context) {
        return request(title, message, {
            "buttons": ["ok"],
            "defaultButton": "ok",
            "context": context || {},
            "severity": "info"
        })
    }

    function warning(title, message, context) {
        return request(title, message, {
            "buttons": ["ok"],
            "defaultButton": "ok",
            "context": context || {},
            "severity": "warning"
        })
    }

    function error(title, message, context) {
        return request(title, message, {
            "buttons": ["ok"],
            "defaultButton": "ok",
            "context": context || {},
            "severity": "error"
        })
    }

    function confirm(title, message, context, defaultButton) {
        return request(title, message, {
            "buttons": ["ok", "cancel"],
            "defaultButton": defaultButton || "cancel",
            "context": context || {}
        })
    }

    function yesNo(title, message, context, defaultButton) {
        return request(title, message, {
            "buttons": ["yes", "no"],
            "defaultButton": defaultButton || "no",
            "context": context || {}
        })
    }

    function destructive(title, message, acceptButton, context) {
        const button = acceptButton || "discard"
        return request(title, message, {
            "buttons": [button, "cancel"],
            "defaultButton": "cancel",
            "destructiveButton": button,
            "context": context || {}
        })
    }

    function request(title, message, options) {
        if (!coordinator)
            return ""
        return coordinator.requestDialog(title || "", message || "", options || {})
    }
}
