#ifndef GAMNEMONIC_H
#define GAMNEMONIC_H

#include <QString>
#include <QVector>
#include <QList>
#include <QByteArray>
#include "gaparameter.h"
#include "gaparser.h"

/* This represents both a potential instruction's definition in both
 * machine and assembly language.  You won't find the parameters in this
 * header, but rather in the GAParameterGroup definition.
 *
 * If we do our job right, we can translate in both directions.
 *
 *
 * For most languages, the length is the accurate length of the
 * whole instruction.  8086 and friends are bonkers, so for those,
 * see the instructions to understand how the MODRM byte region
 * expands.
 */

class GAInstruction;
class GAParameter;
class GALanguage;

class GAMnemonic : public GAParameterGroup
{
public:
    //Simple constructor.
    GAMnemonic(QString mnemonic, uint32_t length,
               const char *opcode,

               //Optional arguments default to null.
               const char *opcodemask=0,
               const char *invertmask=0);
    //Even simpler, just a mnemonic for a byte.
    GAMnemonic(QString mnemonic, const char *opcode);


    //Prioritize this over other mnemonics in disassembly.
    GAMnemonic* prioritize(int priority=1);
    int priority=0;

    //Reject match if specified field is zero (for instruction disambiguation).
    GAMnemonic* rejectWhenZero(const char* mask);
    QList<QByteArray> rejectZeroMasks;


    //Short help string for the cheat sheet.
    GAMnemonic* help(QString help);
    //Short example string for self testing.
    GAMnemonic* example(QString example);
    //Mask of bits for which you don't care.
    void dontcare(const char *mask=0);

    //Does the Mnemonic match bytes?  If so, decode it.
    virtual int match(GAInstruction &ins, uint64_t adr, uint32_t &len,
                      const char *bytes);
    //Does the Mnemonic match parameters?  If so, encode it.
    virtual int match(GAInstruction &ins, uint64_t adr, QString verb,
                      QList<GAParserOperand> ops);

    //Unique name.
    QString name="undefined";
    //Cheat sheet string.
    QString helpstr="";
    //Example instruction.  Will be used for self testing.
    QString examplestr="";

    //Pointer to the language.
    GALanguage *lang=0;

    //These used to be private.
    uint32_t length=0;
    char opcode[GAMAXLEN]; //Opcode bytes.
    char dcmask[GAMAXLEN]; //Don't-care mask.

private:
    void config(QString mnemonic, uint32_t length,
                const char *opcode,
                const char *opcodemask, const char *invertmask);
    //Helper to check reject-when-zero constraints.
    bool passesRejectConstraints(const char* bytes);
};

#endif // GAMNEMONIC_H
