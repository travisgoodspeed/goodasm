#include "QDebug"
#include <stdlib.h>
#include <string.h>

#include "gamnemonic.h"
#include "gainstruction.h"
#include "gaparameter.h"
#include "galanguage.h"


void GAMnemonic::config(QString mnemonic, uint32_t length,
                        const char *opcode, const char *opcodemask,
                        const char *invertmask){
    assert(length<=GAMAXLEN);

    memset(dcmask, 0, GAMAXLEN); //Zero the don't-care mask.

    //Assign the private variables.
    this->name=mnemonic;
    this->length=length;
    memcpy(this->opcode, opcode, GAMAXLEN);

    if(opcodemask)
        memcpy(this->opcodemask, opcodemask, GAMAXLEN);
    else
        memset(this->opcodemask, 0, GAMAXLEN);

    if(invertmask)
        memcpy(this->invertmask, invertmask, GAMAXLEN);
    else //Without a mask, we take all bytes.
        memset(this->invertmask, 0, GAMAXLEN);

    //Sanity checks so we don't crash.
    assert(this->name.length()>0);
    assert(this->length);
    assert(this->opcode);
    assert(this->opcodemask);
    assert(this->invertmask);

    return;
}


//Mask of bits for which you don't care.
void GAMnemonic::dontcare(const char *mask){
    for(int i=0; i<GAMAXLEN; i++)
        dcmask[i]|=mask[i];
}

//Does the Mnemonic match bytes?  If so, decode bytes to an instruction.
int GAMnemonic::match(GAInstruction &ins, uint64_t adr, uint32_t &len,
                      const char *bytes){
    assert(length); //No zero byte mnemonics.

    //Don't match on a misaligned byte.
    if(adr%lang->align != 0)
        return 0;

    //qDebug()<<"Length="<<this->length;
    //This fails out if any byte does not equal the opcode, modulo masking.
    for(int i=0; i<length; i++){
        if((bytes[i]&opcodemask[i])!=opcode[i])
            return 0; //No match.
    }

    //Check reject-when-zero constraints (e.g., RISC-V c.addi requires rd != 0).
    if(!passesRejectConstraints(bytes))
        return 0;

    //Here we have a match, but we need to form a valid instructions.
    ins.verb=name;
    ins.type=GAInstruction::MNEMONIC;
    ins.len=len=this->length;
    ins.helpstr=this->helpstr;

    ins.params="";  //Resets the decoding.
    for(int i=0; i<params.count(); i++){
        ins.params+=params[i]->decode(lang, adr, bytes, len);
        if(i+1<params.count()) ins.params+=", ";
    }

    //Set the back pointer.
    ins.mnem=this;

    //Set the data as a QByteArray with the right length.
    QByteArray d;
    for(int i=0; i<len; i++)
        d.append(bytes[i]);
    ins.data=d;
    assert(ins.len=ins.data.length());

    return 1;
}

//Does the Mnemonic match source?  If so, encode it to bytes.
int GAMnemonic::match(GAInstruction &ins, uint64_t adr,
                      QString verb, QList<GAParserOperand> ops){
    //Names and parameter count must match.
    if(this->name!=verb || ops.count()!=params.count())
        return 0;

    if(ins.len!=0 && ins.len<length)
        return 0;


    int i=0;
    foreach (auto param, params) {
        GAParserOperand op=ops[i];
        //Use mnemonic length because instruction length is not yet set!
        bool match=param->match(&op, length);
        if(!match) return 0;
        i++;
    }

    //FIXME: We're in trouble if the match is a mistake.
    //Registers might be mistaken for values.

    ins.len=length;
    ins.type=ins.MNEMONIC;
    ins.verb=verb;

    QByteArray bytes;
    QString opstring="";
    //First apply opcode.
    for(i=0; i<length; i++)
        bytes.append(opcode[i]);
    //Then apply parameters
    i=0;
    foreach (auto param, params) {
        GAParserOperand op=ops[i++];
        param->encode(lang, adr, bytes, op, ins.len);

        if(i>1) opstring+=", ";
        opstring+=op.render();
    }

    //Finally, set the bytes.
    ins.data=bytes;
    assert(bytes.length()==ins.len);
    ins.params=opstring;
    //Set the back pointer.
    ins.mnem=this;
    //Disassemble for listings.
    //match(ins, adr, length, ins.data.constData()); //Breaks stuff.
    return 1;
}

GAMnemonic::GAMnemonic(QString mnemonic, uint32_t length,
                       const char *opcode, const char *opcodemask,
                       const char *invertmask) {
    config(mnemonic, length, opcode, opcodemask, invertmask);
}


//Even simpler, just a mnemonic for a lone byte.
GAMnemonic::GAMnemonic(QString mnemonic, const char *opcode){
    config(mnemonic, 1, opcode, "\xff", 0);
    //This quite likely is its own example!
    example(mnemonic);
}


//Short help string for the cheat sheet.
GAMnemonic* GAMnemonic::help(QString help){
    this->helpstr=help;
    return this;
}
//Example string, used both for help and for self-testing.
GAMnemonic* GAMnemonic::example(QString example){
    this->examplestr=example;
    return this;
}
//Prioritize this over other mnemonics in disassembly.
GAMnemonic* GAMnemonic::prioritize(int priority){
    this->priority=priority;
    return this;
}

//Reject match if specified field is zero (for instruction disambiguation).
GAMnemonic* GAMnemonic::rejectWhenZero(const char* mask){
    QByteArray m;
    for(uint32_t i = 0; i < length; i++)
        m.append(mask[i]);
    rejectZeroMasks.append(m);
    return this;
}

//Helper to check reject-when-zero constraints.
bool GAMnemonic::passesRejectConstraints(const char* bytes){
    for(const auto& rzMask : rejectZeroMasks) {
        bool fieldIsZero = true;
        for(uint32_t i = 0; i < length && fieldIsZero; i++) {
            if(bytes[i] & rzMask[i])
                fieldIsZero = false;
        }
        if(fieldIsZero)
            return false;  // Reject: field must be non-zero
    }
    return true;
}
