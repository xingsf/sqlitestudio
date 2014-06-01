#ifndef IMPORTMANAGER_H
#define IMPORTMANAGER_H

#include "pluginservicebase.h"
#include <QFlags>
#include <QStringList>

class ImportPlugin;
class Db;
class CfgEntry;

class ImportManager : public PluginServiceBase
{
        Q_OBJECT

    public:
        struct StandardImportConfig
        {
            /**
             * @brief Text encoding.
             *
             * Always one of QTextCodec::availableCodecs().
             * Codec is important for text-based data. For binary data it should irrelevant to the import plugin.
             */
            QString codec;

            /**
             * @brief Name of the file that the import is being done from.
             *
             * This is provided just for information to the import process,
             * but the plugin should use data stream provided to each called import method,
             * instead of opening the file from this name.
             *
             * It will be null string if importing is not performed from a file, but from somewhere else
             * (for example from a clipboard).
             */
            QString inputFileName;
        };

        enum StandardConfigFlag
        {
            CODEC             = 0x01, /**< Text encoding (see StandardImportConfig::codec). */
            FILE_NAME         = 0x02, /**< Input file (see StandardImportConfig::inputFileName). */
        };

        Q_DECLARE_FLAGS(StandardConfigFlags, StandardConfigFlag)

        ImportManager();

        QStringList getImportDataSourceTypes() const;
        ImportPlugin* getPluginForDataSourceType(const QString& dataSourceType) const;

        void configure(const QString& dataSourceType, const StandardImportConfig& config);
        void importToTable(Db* db, const QString& table);

        static bool isAnyPluginAvailable();

    private:
        StandardImportConfig importConfig;
        ImportPlugin* plugin = nullptr;
        bool importInProgress = false;
        Db* db = nullptr;
        QString table;

    public slots:
        void interrupt();

    private slots:
        void finalizeImport(bool result);
        void handleTableCreated(Db* db, const QString& table);

    signals:
        void importFinished();
        void importSuccessful();
        void importFailed();
        void orderWorkerToInterrupt();
        void schemaModified(Db* db);
};

#define IMPORT_MANAGER SQLITESTUDIO->getImportManager()

Q_DECLARE_OPERATORS_FOR_FLAGS(ImportManager::StandardConfigFlags)

#endif // IMPORTMANAGER_H
