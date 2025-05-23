#ifndef GASYMBOLTABLE_H
#define GASYMBOLTABLE_H

#include <QVector>
#include <QMap>
#include <QString>

class GALanguage;

/* This class holds all of the symbols of the project.
 * Some are generated by labels and others are generated
 * by label statements in the code.
 *
 * The table is "complete" if every symbol that is ever
 * consumed is also defined.  We reach "closure" if the
 * hash of all symbols and values is the same in two
 * subsequent assembly runs, but that can't be checked
 * from this class alone without repeating a rendering.
 *
 *
 */

class GASymbol {
public:
    GASymbol(QString n);
    QString name="undefined";    // Name of the symbol
    uint64_t absval=0;           // Absolute address.
    bool defined=false;          // Turns true when a value is applied.
    bool referenced=false;       // Has the value been used, not just matched?
private:
    //void addReference(uint64_t adr); //Adds a reference from an address.
};

class GASymbolTable {
public:
    GASymbolTable();
    GASymbol* setSymbol(QString name, uint64_t absval);      // Sets a symbol to a number.
    GASymbol* setSymbol(QString name, QString value);        // Sets a symbol to a string of a number.
    GASymbol* findSymbol(uint64_t absval);                   // Find a symbol by address.
    GASymbol* findSymbol(QString name, bool autogen=false);  // Find a symbol by name.
    QString exportTable();
    QVector<GASymbol*> sorted();    //Returns sorted table of links to symbols in linear order.
    QStringList completions(QString prefix);
    void clear();                   // Clear all definitions between runs.
    QByteArray symbolhash();        // Unique hash of symbols, used to test for closure.
    uint64_t count();               // How many symbols are defined?
    bool complete();                // Have all used symbols been defined?
    QStringList missingSymbols();   // Which symbols are used but not defined?

    GALanguage *lang=0;
    QMap<QString, GASymbol*> table;       // Symbols by name.
    QMap<uint64_t, GASymbol*> tableByAdr; // Symbols by address.
private:
};

#endif // GASYMBOLTABLE_H
