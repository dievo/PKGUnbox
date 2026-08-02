#include "toml_parser.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

QMap<QString, QString> TomlParser::parseFile(const QString &filePath) {
    QMap<QString, QString> config;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return config;
    }

    QTextStream in(&file);
    QString currentSection;

    // Regex for section: [SectionName]
    QRegularExpression sectionRegex(R"(^\s*\[([^\]]+)\]\s*$)");
    // Regex for key = value (supports quoted strings and bare values)
    QRegularExpression keyValueRegex(R"(^\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*\"?([^\"\n]+?)\"?\s*$)");

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        // Skip empty lines and comments
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        // Check for section
        QRegularExpressionMatch sectionMatch = sectionRegex.match(line);
        if (sectionMatch.hasMatch()) {
            currentSection = sectionMatch.captured(1).trimmed();
            continue;
        }

        // Check for key = value
        QRegularExpressionMatch kvMatch = keyValueRegex.match(line);
        if (kvMatch.hasMatch()) {
            QString key = kvMatch.captured(1).trimmed();
            QString value = kvMatch.captured(2).trimmed();

            // Store as section.key
            QString fullKey = currentSection.isEmpty() ? key : currentSection + "." + key;
            config[fullKey] = value;
        }
    }

    file.close();
    return config;
}

QString TomlParser::getValue(const QMap<QString, QString> &config,
                            const QString &section,
                            const QString &key) {
    QString fullKey = section + "." + key;
    return config.value(fullKey, QString());
}
