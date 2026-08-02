#pragma once

#include <QString>
#include <QMap>

/**
 * Simple TOML parser for reading shadPS4 config.toml
 * Only supports basic key = "value" pairs in sections
 */
class TomlParser {
public:
    /**
     * Parse a TOML file and return a map of section.key -> value
     * Example: [General]\nfoo = "bar" -> {"General.foo": "bar"}
     */
    static QMap<QString, QString> parseFile(const QString &filePath);

    /**
     * Get a value from parsed config
     * @param config Parsed config map
     * @param section Section name (e.g., "General")
     * @param key Key name (e.g., "game_install_dir")
     * @return Value or empty string if not found
     */
    static QString getValue(const QMap<QString, QString> &config,
                           const QString &section,
                           const QString &key);
};
