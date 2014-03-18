#include "sqlitestudio.h"
#include "plugins/plugin.h"
#include "services/pluginmanager.h"
#include "common/utils.h"
#include "common/utils_sql.h"
#include "completionhelper.h"
#include "parser/keywords.h"
#include "parser/lexer.h"
#include "services/notifymanager.h"
#include "plugins/sqlformatterplugin.h"
#include "sqlformatter.h"
#include "plugins/generalpurposeplugin.h"
#include "plugins/sqlfunctionplugin.h"
#include "plugins/dbplugin.h"
#include "common/unused.h"
#include "services/functionmanager.h"
#include "plugins/scriptingplugin.h"
#include "plugins/scriptingqt.h"
#include "plugins/dbpluginsqlite3.h"
#include "services/impl/configimpl.h"
#include "services/impl/dbmanagerimpl.h"
#include "services/impl/functionmanagerimpl.h"
#include "services/impl/pluginmanagerimpl.h"
#include "impl/dbattacherimpl.h"
#include <QProcessEnvironment>
#include <QThreadPool>

DEFINE_SINGLETON(SQLiteStudio)

SQLiteStudio::SQLiteStudio()
{
}

SQLiteStudio::~SQLiteStudio()
{
    cleanUp();
}

void SQLiteStudio::parseCmdLineArgs()
{
    for (int i = 0; i < cmdLineArgs.size(); i++)
    {
        if (cmdLineArgs[i] == "-d")
        {
            // TODO
        }
    }
}
DbAttacherFactory* SQLiteStudio::getDbAttacherFactory() const
{
    return dbAttacherFactory;
}

void SQLiteStudio::setDbAttacherFactory(DbAttacherFactory* value)
{
    safe_delete(dbAttacherFactory);
    dbAttacherFactory = value;
}

PluginManager* SQLiteStudio::getPluginManager() const
{
    return pluginManager;
}

void SQLiteStudio::setPluginManager(PluginManager* value)
{
    safe_delete(pluginManager);
    pluginManager = value;
}

FunctionManager* SQLiteStudio::getFunctionManager() const
{
    return functionManager;
}

void SQLiteStudio::setFunctionManager(FunctionManager* value)
{
    safe_delete(functionManager);
    functionManager = value;
}

DbManager* SQLiteStudio::getDbManager() const
{
    return dbManager;
}

void SQLiteStudio::setDbManager(DbManager* value)
{
    safe_delete(dbManager);
    dbManager = value;
}

Config* SQLiteStudio::getConfig() const
{
    return config;
}

void SQLiteStudio::setConfig(Config* value)
{
    safe_delete(config);
    config = value;
}


SqlFormatter *SQLiteStudio::getSqlFormatter() const
{
    return sqlFormatter;
}

void SQLiteStudio::init(const QStringList& cmdListArguments)
{
    env = new QProcessEnvironment(QProcessEnvironment::systemEnvironment());

    QThreadPool::globalInstance()->setMaxThreadCount(10);

    initUtils();
    CfgMain::staticInit();
    Db::metaInit();
    initUtilsSql();
    initKeywords();
    Lexer::staticInit();
    CompletionHelper::init();

    qRegisterMetaType<ScriptingPlugin::Context*>();

    NotifyManager::getInstance();

    dbAttacherFactory = new DbAttacherDefaultFactory();

    config = new ConfigImpl();
    config->init();

    pluginManager = new PluginManagerImpl();
    dbManager = new DbManagerImpl();

    pluginManager->registerPluginType<GeneralPurposePlugin>(QObject::tr("General purpose"));
    pluginManager->registerPluginType<DbPlugin>(QObject::tr("Database support"));
    pluginManager->registerPluginType<SqlFormatterPlugin>(QObject::tr("SQL formatter"), "formatterPluginsPage");
    pluginManager->registerPluginType<SqlFunctionPlugin>(QObject::tr("SQL function"));
    pluginManager->registerPluginType<ScriptingPlugin>(QObject::tr("Scripting languages"));

    sqlFormatter = new SqlFormatter();
    connect(CFG_CORE.General.ActiveSqlFormatter, SIGNAL(changed(QVariant)), this, SLOT(updateSqlFormatter()));
    connect(pluginManager, SIGNAL(pluginsInitiallyLoaded()), this, SLOT(updateSqlFormatter()));

    // FunctionManager needs to be set up before DbManager, cause when DbManager starts up, databases make their
    // connections and register functions.
    functionManager = new FunctionManagerImpl();

    cmdLineArgs = cmdListArguments;

    connect(pluginManager, SIGNAL(pluginsInitiallyLoaded()), DBLIST, SLOT(loadDbListFromConfig()));

    pluginManager->loadBuiltInPlugin(new ScriptingQt);
    pluginManager->loadBuiltInPlugin(new DbPluginSqlite3);

    pluginManager->init();

    connect(pluginManager, SIGNAL(loaded(Plugin*,PluginType*)), this, SLOT(pluginLoaded(Plugin*,PluginType*)));
    connect(pluginManager, SIGNAL(aboutToUnload(Plugin*,PluginType*)), this, SLOT(pluginToBeUnloaded(Plugin*,PluginType*)));
    connect(pluginManager, SIGNAL(unloaded(QString,PluginType*)), this, SLOT(pluginUnloaded(QString,PluginType*)));

    parseCmdLineArgs();
}

void SQLiteStudio::cleanUp()
{
    safe_delete(functionManager);
    safe_delete(dbManager);
    safe_delete(config);
    safe_delete(pluginManager);
    safe_delete(sqlFormatter);
    safe_delete(dbAttacherFactory);
    safe_delete(env);
    NotifyManager::destroy();
}

void SQLiteStudio::updateSqlFormatter()
{
    QList<SqlFormatterPlugin *> sqlFormatterPlugins = PLUGINS->getLoadedPlugins<SqlFormatterPlugin>();
    QString activeFormatterName = CFG_CORE.General.ActiveSqlFormatter.get();
    foreach (SqlFormatterPlugin* plugin, sqlFormatterPlugins)
    {
        if (plugin->getName() == activeFormatterName)
        {
            sqlFormatter->setFormatter(plugin);
            return;
        }
    }
    sqlFormatter->setFormatter(nullptr);
}

void SQLiteStudio::pluginLoaded(Plugin* plugin, PluginType* pluginType)
{
    UNUSED(plugin);
    if (pluginType->isForPluginType<SqlFormatterPlugin>())
        updateSqlFormatter();
}

void SQLiteStudio::pluginToBeUnloaded(Plugin* plugin, PluginType* pluginType)
{
    UNUSED(plugin);
    UNUSED(pluginType);
}

void SQLiteStudio::pluginUnloaded(const QString& pluginName, PluginType* pluginType)
{
    UNUSED(pluginName);
    if (pluginType->isForPluginType<SqlFormatterPlugin>())
        updateSqlFormatter();
}

QString SQLiteStudio::getEnv(const QString &name, const QString &defaultValue)
{
    return env->value(name, defaultValue);
}

DbAttacher* SQLiteStudio::createDbAttacher(Db* db)
{
    return dbAttacherFactory->create(db);
}
