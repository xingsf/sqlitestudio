#ifndef FUNCTIONSEDITOR_H
#define FUNCTIONSEDITOR_H

#include "mdichild.h"
#include "common/extactioncontainer.h"
#include "services/config.h"
#include "services/functionmanager.h"
#include <QItemSelection>
#include <QSortFilterProxyModel>

namespace Ui {
class FunctionsEditor;
}

class FunctionsEditorModel;
class SqlFunctionPlugin;
class SyntaxHighlighterPlugin;
class DbTreeItem;
class QTreeWidgetItem;
class QSyntaxHighlighter;
class SelectableDbModel;

class FunctionsEditor : public MdiChild, public ExtActionContainer
{
    Q_OBJECT

    public:
        enum Action
        {
            COMMIT,
            ROLLBACK,
            ADD,
            DELETE,
            ARG_ADD,
            ARG_EDIT,
            ARG_DEL,
            ARG_MOVE_UP,
            ARG_MOVE_DOWN,
            HELP
        };

        explicit FunctionsEditor(QWidget *parent = 0);
        ~FunctionsEditor();

        bool restoreSessionNextTime();

    protected:
        QVariant saveSession();
        bool restoreSession(const QVariant &sessionValue);
        QString getIconNameForMdiWindow();
        QString getTitleForMdiWindow();
        void createActions();
        void setupDefShortcuts();

    private:
        void init();
        int getCurrentFunctionRow() const;
        void functionDeselected(int row);
        void functionSelected(int row);
        void clearEdits();
        void selectFunction(int row);
        QModelIndex getSelectedArg() const;
        QStringList getCurrentArgList() const;
        QStringList getCurrentDatabases() const;
        FunctionManager::Function::Type getCurrentFunctionType() const;

        Ui::FunctionsEditor *ui;
        FunctionsEditorModel* model = nullptr;
        QSortFilterProxyModel* functionFilterModel = nullptr;
        bool currentModified = false;
        QHash<QString,SqlFunctionPlugin*> functionPlugins;
        QHash<QString,SyntaxHighlighterPlugin*> highlighterPlugins;
        SelectableDbModel* dbListModel = nullptr;
        QString currentHighlighterLang;
        QSyntaxHighlighter* currentMainHighlighter = nullptr;
        QSyntaxHighlighter* currentFinalHighlighter = nullptr;
        QSyntaxHighlighter* currentInitHighlighter = nullptr;

    private slots:
        void commit();
        void rollback();
        void newFunction();
        void deleteFunction();
        void updateModified();
        void updateState();
        void updateCurrentFunctionState();
        void functionSelected(const QItemSelection& selected, const QItemSelection& deselected);
        void validateName();
        void addFunctionArg();
        void editFunctionArg();
        void delFunctionArg();
        void moveFunctionArgUp();
        void moveFunctionArgDown();
        void updateArgsState();
        void applyFilter(const QString& value);
        void help();
};

#endif // FUNCTIONSEDITOR_H
