#include <QDebug>
#include <QFile>
#include <QRandomGenerator>
#include <iostream>


#include "goodasm.h"
#include "galanguage.h"
#include "gainstruction.h"
#include "galexer.h"
#include "gaparser.h"

//Languages
#include "galang8086.h"
#include "galangucom43.h"
#include "galangtlcs47.h"
#include "galangs2000.h"
#include "galangpic16c5x.h"
#include "galangmarc4.h"
#include "galang6502.h"
#include "galang6805.h"
#include "galangst7.h"
#include "galang8051.h"
#include "galangsm83.h"
#include "galangchip8.h"
#include "galangz80.h"
#include "galangz8.h"
#include "galangfcard.h"
#include "galang8080.h"
#include "galangh83.h"
#include "galangarm7tdmi.h"
#include "galangtms320c28x.h"
#include "galangti80.h"
#include "galangriscv32.h"

//Listings
#include "galistingdefault.h"
#include "galistingnasm.h"
#include "galistingc.h"
#include "galistinggo.h"
#include "galistingyarax.h"
#include "galistingmarkdown.h"
#include "galistinghex.h"
#include "galistingga.h"

//Graders
#include "gagradervalidops.h"
#include "gagraderjumptargets.h"
#include "gagradergameboy.h"
#include "gagraderz80.h"
#include "gagradermovtarget.h"
#include "gagrader8051jmptable.h"
#include "gagrader8051pushpop.h"

//Symbol table.
#include "gasymboltable.h"


bool GoodASM::setLanguage(QString language){
    //Load the languages, in case we need them.
    if(languages.empty()){
        languages.append(new GALang8086());
        languages.append(new GALangUcom43());
        languages.append(new GALangTLCS47());
        languages.append(new GALangS2000()); //EMZ1001
        languages.append(new GALangPIC16C5x());
        languages.append(new GALangMARC4());
        languages.append(new GALang6502());
        languages.append(new GALang6805());
        languages.append(new GALangST7());
        languages.append(new GALang8051());
        languages.append(new GALangSM83());  //GameBoy.
        languages.append(new GALangChip8());
        languages.append(new GALangZ80());
        languages.append(new GALangZ8());
        languages.append(new GALangFCard());
        languages.append(new GALang8080());
        languages.append(new GALangH83());
        languages.append(new GALangARM7TDMI());
        languages.append(new GALangTMS320C28x());
        languages.append(new GALangTI80());
        languages.append(new GALangRISCV());
    }

    //Does our language match?
    lang=0;
    foreach (auto lang, languages) {
        if(lang->name==language)
            setLanguage(lang);
    }

    //If it's empty, that's not an unexpected problem.
    if(language=="")
        setLanguage(0);

    //Empty language if we need one.
    if(!lang){
        setLanguage(0);
        qDebug()<<"Language "<<language<<"is not known.";
        return false;
    }

    //True if the lang has been set.
    return true;
}

//List of supported language names.
QVector<QString> GoodASM::languageNames(){
    QVector<QString> v;
    if(languages.empty())
        setLanguage("");
    foreach(auto l, languages){
        v.append(l->name);
    }
    return v;
}


void GoodASM::setListing(QString style){
    if(listings.empty()){
        //Listing styles, for formatting text output.
        listings.append(new GAListingDefault());
        listings.append(new GAListingNasm());
        listings.append(new GAListingC());
        listings.append(new GAListingGo());
        listings.append(new GAListingYaraX());
        listings.append(new GAListingMarkdown());
        listings.append(new GAListingHex());
        listings.append(new GAListingGA());
    }
    listing=0;
    foreach(auto l, listings){
        if(l->name==style)
            setListing(l);
    }

    //You really need a name to your listing.
    assert(listing);
}


void GoodASM::setGrader(QString grader){
    if(graders.empty()){
        //Graders, for identifying unknown languages or confirming guesses.
        graders.append(new GAGraderValidOps());
        graders.append(new GAGraderJumpTargets());
        graders.append(new GAGraderGameBoy());
        graders.append(new GAGraderMovTarget());
        graders.append(new GAGraderZ80());
        graders.append(new GAGrader8051JmpTable());
        graders.append(new GAGrader8051PushPop());
    }
    this->grader=0;
    foreach(auto g, graders){
        if(g->name==grader)
            setGrader(g);
    }
}
void GoodASM::setGrader(GAGrader *grader){
    this->grader=grader;
}



GoodASM::GoodASM(QString language){
    setLanguage(language);
    setListing("default");
    assert(lang);
    symbols.lang=lang;
}

GoodASM::GoodASM(GALanguage *language) {
    //Parameter is language, or maybe null.
    setLanguage(language);
    setListing("default");
    assert(lang);
    symbols.lang=lang;
}

QString GoodASM::opcodeTable(){
    /* Here, i is the high nybble and j is the low nybble.
     * We are disassembling a short buffer with the first
     * byte set to IJ and then including its verb name
     * in an ASCII table for the user's convenience.
     */

    QString toret="";

    char nyb[]="0123456789abcdef";

    //Top row header.
    toret+="  \t";
    for(int j=0; j<0x10; j++){
        toret+="0";
        toret+=QChar(nyb[j]);
        toret+="\t";
    }
    toret+="\n";


    for(int i=0; i<0x10; i++){
        toret+=QChar(nyb[i]);
        toret+="0\t";
        for(int j=0; j<0x10; j++){
            QByteArray a;
            QString name="?";
            a.append(((i<<4)|j)&0xff);
            a.append((char) 0);
            a.append((char) 0);
            a.append((char) 0);
            clear();
            load(a);

            if(at(0).mnem)
                name=at(0).mnem->name;
            toret+=name+"\t";
        }
        toret+="\n";
    }
    return toret;
}


GoodASM::~GoodASM(){
    //Free the selected language if it's not in the list.
    if(lang && !languages.contains(lang))
        delete lang;
    //Free the potential languages.
    foreach(auto l, languages)
        delete l;
}

//Returns filename and line number.
QString GoodASM::addr2line(int64_t adr){
    return QString("%1:%2").arg(filename).arg(line);
}
//Logs an error at current file and line.
void GoodASM::error(QString message){
    errors.append(addr2line()+"\t"+message);
}
//Prints all errors from the latest run.
int GoodASM::printErrors(){
    for(int i=0; i<errors.count(); i++){
        std::cout<<errors[i].toStdString()<<"\n";
    }
    return errors.count();
}
//Clears errrors.  Call this between runs.
void GoodASM::clearErrors(){
    errors.clear();
}

//Formats a source line for printing.
QString GoodASM::formatSource(QString label, QString code,
                              QString comment, QString comment2){
    assert(listing);
    return listing->formatSource(label, code, comment, comment2);
}

//New complication array, for REPLXX.
QVector<QString> GoodASM::completions(QString line){
    QVector<QString> vector;
    QString fragment=line;  //FIXME: Should be last word.
    assert(lang);



    //First word is an exact match to a mnemonic, complete example.
    QStringList words=line.split(" ");
    if(words.length()==2){
        //Just one word, maybe exactly matches some mnemonics?
        QString v=words[0];
        foreach(auto m, lang->mnemonics){
            if((line==m->name+" " || line==m->name)){
                vector.append(m->examplestr);
            }
        }
        vector.removeDuplicates();
        vector.sort();
        if(vector.length()>0){
            // Fake entry, so that the completion isn't taken.
            vector.append(" ");
            return vector;
        }
    }

    //Completes the last word as a symbol, mnemonic or register.
    fragment=words[words.length()-1]; //Completing last word.
    foreach(auto m, lang->mnemonics)
        if(m->name.startsWith(fragment))
            vector.append(m->name);
    foreach(auto m, lang->regnames)
        if(m.startsWith(fragment))
            vector.append(m);
    vector.append(symbols.completions(fragment));
    vector.removeDuplicates();
    vector.sort();

    return vector;
}


bool GoodASM::setLanguage(GALanguage *language){
    lang=language;
    if(!lang) //Apply if it's an empty language.
        lang=new GALanguage();
    lang->setGoodASM(this);
    assert(lang);


    //Reload binary if we are disassembling.
    if(type==DISASSEMBLY)
        load(bytes);

    return true;
}


void GoodASM::setListing(GAListing *style){
    listing=style;
    if(!listing) //Apply if empty language.
        listing=new GAListingDefault();
    assert(listing);
}


// Text input or disassembly.
QString GoodASM::source(){
    assert(listing);

    listing->render(this);           //First pass sets column widths.
    return(listing->render(this));   //Second is real.
}

// All the self testing.
bool GoodASM::selftest_all(){
    bool toret=
        selftest_examples()
                 && selftest_collisions()
                 && selftest_overlap()
                 && selftest_length();
    return toret;
}


// Are the mask strings null-terminated?
bool GoodASM::selftest_length(){
    foreach(auto m, lang->mnemonics){
        uint64_t l=m->length;
        if(m->opcodemask[l]){
            qDebug()<<"Mask length is suspicious: "<<m->examplestr;
            return false;
        }
    }

    return true;
}

// Do disassemblies ever violate assertions?
bool GoodASM::selftest_fuzz(){
    qDebug()<<"Fuzz testing the disassembler.";
    assert(lang);

    QRandomGenerator *rnd=QRandomGenerator::system();

    for(int i=0; i<0x10000; i++){
        uint32_t val=rnd->generate();
        QByteArray a;
        a.append(val&0xff);
        a.append((val>> 8)&0xff);
        a.append((val>>16)&0xff);
        a.append((val>>24)&0xff);
        clear();
        load(a);
        source();
        if(duplicates>0){
            qDebug()<<"Duplicates found in fuzzing.";
            qDebug()<<hexdump();
            return false;
        }
    }

    return true; //Haven't crashed yet!
}

// Do the examples line up?
bool GoodASM::selftest_examples(){
    uint64_t passes=0;
    uint64_t fails=0;
    qDebug()<<"Testing each example in the language.";
    assert(lang);

    foreach(auto m, lang->mnemonics){
        QString lineinput=m->examplestr;
        clear(true);
        load(lineinput);
        if(instructions.length()>0
            && instructions[0].mnem
            && instructions[0].mnem==m
            ){
            //qDebug()<<"TEST: "<<lineinput;
            passes++;

            //Must come before we attempt disassmbly, or we'll miss the missing symbols.
            if(!symbols.complete()){
                qDebug()<<"FAIL: "<<lineinput;
                qDebug()<<"Undefined symbols: "<<symbols.missingSymbols();
                fails++;
            }

            QByteArray a=bytes;
            load(a);
            source();
            if(instructions.length()>0
                && instructions[0].mnem
                && (instructions[0].mnem==m
                    || instructions[0].mnem->priority>m->priority)){
                /* Either reflection works here, or the returned mnemonic is higher priority
                 * than the source mnemonic.  See ASL and LSL in 6805 for an example of that.
                 */
            }else{
                qDebug()<<"Reflection failed: "
                         //<<instructions[0].mnem->name
                         <<m->name
                         <<bytes;
                fails++;
            }

            if(duplicates>0){
                qDebug()<<"DUPE: "<<lineinput;
                fails++;
            }


        }else{
            qDebug()<<"FAIL: "
                     <<(lineinput!=""?lineinput:m->name);
            fails++;
        }
        clear();
    }

    return(!fails);
}

// Are there collisions?
bool GoodASM::selftest_collisions(){
    qDebug()<<"Testing for mask collisions.";
    assert(lang);

    foreach(auto m, lang->mnemonics){
        foreach(auto n, lang->mnemonics){
            if(m!=n){
                //TODO: Check the collision of masks here.

            }
        }
    }

    return true;
}


// Does a parameter's mask overlap with the opcode?
bool GoodASM::selftest_overlap(){
    qDebug()<<"Testing for mask overlaps.";
    assert(lang);

    foreach(auto m, lang->mnemonics){
        //Working mask, to ensure all bits are used.
        uint8_t wmask[GAMAXLEN];
        memcpy(wmask, m->opcodemask, m->length);
        foreach(auto p, m->params){
            for(int i=0; i<m->length; i++){
                wmask[i]|=p->mask[i];
                if(m->opcodemask[i] & p->mask[i]){
                    qDebug()<<"Mask overlap: "<<m->examplestr;
                    qDebug()<<"Byte"<<i<<": "<<Qt::hex<<(uint8_t)(m->opcodemask[i] & p->mask[i]);
                    return false;
                }
            }
        }
        for(int i=0; i<m->length; i++){
            if(wmask[i]&m->dcmask[i]){
                qDebug()<<"Consumed don't-care bit: "<<m->examplestr;
                qDebug()<<"Byte"<<i<<": "<<Qt::hex<<(uint8_t) (wmask[i]&m->dcmask[i]);
                return false;
            }
            if(((uint8_t) wmask[i]|(uint8_t) m->dcmask[i])!=0xff){
                qDebug()<<"Empty mask bit: "<<m->examplestr;
                qDebug()<<"Byte"<<i<<": "<<Qt::hex<<(((uint8_t) wmask[i]|(uint8_t) m->dcmask[i]))
                         <<"="<<(uint8_t) wmask[i]<<"|"<<(uint8_t) m->dcmask[i];
                return false;
            }
        }
    }
    return true;
}


// Clears all code or data.
void GoodASM::clear(bool symbols){
    instructions.clear();
    bytes.clear();
    //We don't clear damage.  Instead, it persists.
    clearErrors();
    if(symbols) this->symbols.clear();
    workingadr=baseaddress;
}

// Binary input or assembly.
QByteArray GoodASM::code(){
    //Disassembly is easy, just send back the source bytes.
    if(type==DISASSEMBLY)
        return this->bytes;

    //Assembly is harder.  Walk the instructions and append them.
    if(type==ASSEMBLY){
        QByteArray code;

        foreach(GAInstruction ins, instructions)
            code.append(ins.code());
        return code;
    }

    //Empty output when undefined.
    return QByteArray();
}

// Code in hex.
QString GoodASM::hexdump(){
    QByteArray bytes=code();
    QString hex="";
    int i;
    for(i=0; i<bytes.length(); i++){
        hex.append(QString::asprintf("%02x ",0x00FF &  bytes[i]));

        if(i%32==15)
            hex.append(" ");
        if(i%32==31)
            hex.append("\n");
    }

    if(i%32!=31)
        hex.append("\n");

    return hex;
}

// Load a binary for disassembly.
void GoodASM::load(QByteArray bytes){
    clear(true);  //Clear code and symbols.

    //Load the binary.
    type=DISASSEMBLY;
    this->bytes=bytes;
    duplicates=0;

    workingadr=baseaddress;
    GAInstruction ins=at(baseaddress);
    for(int i=0; ins.adr<baseaddress+bytes.length(); i++){
        append(ins);
        ins=ins.next();
    }
}

// Load a binary of damage.  Call before ::load()
void GoodASM::loadDamage(QByteArray bytes){
    clear(true);  //Clear code and symbols.

    //Load the binary.
    this->damage=bytes;
}


// Disassemble bytes and append them to the working project.
void GoodASM::loadFragment(QString label, QByteArray bytes, QString comment){
    char buf[GAMAXLEN];
    uint64_t base=0;    //Offset into the buffer that we are adding.
    uint32_t len;       //Length of the working instruction.
    while(base<bytes.length()){
        //Load bytes, or zeroes.
        for(int i=0; i<lang->maxbytes; i++)
            buf[i]=(i+base<bytes.length()?bytes.at(i+base):0);
        GAInstruction ins=lang->decode(workingadr, buf, len);
        ins.comment=comment;

        if(label.length()>0){
            ins.label=label;
            label="";
        }

        if(base+len>bytes.length()){
            GAInstruction data=GAInstruction(this, buf[0]);
            data.comment=comment+" (data)";
            if(label.length()>0){
                data.label=label;
                label="";
            }
            base+=1;
            append(data);
        }else{
            base+=len;
            append(ins);
        }
    }

}


// Load source for assembly.
void GoodASM::load(QString source){
    type=ASSEMBLY;

    /* Here we keep parsing the source and abandoning the response
     * until the hash of the symbol table stops changing.
     */
    QByteArray last, next;
    int count=0;
    do{
        last=symbols.symbolhash();   //Has the last symbols.
        clear(false);                //Clear code but not symbols.

        GALexer lex(source);
        GAParser p(&lex, this);
        p.parse();
        next=symbols.symbolhash();
    }while(count++<10 && last!=next);

    if(!symbols.complete())
        error("Symbols incomplete after last pass.");
    if(count>5)
        error(QString("After %1 passes, symbols disagree.").arg(count));

}


//Loads a filename for assembly.
void GoodASM::loadFile(QString file){
    QByteArray bytes;

    QFile input(file);
    if(file=="-")
        input.open(stdin, QIODevice::ReadOnly);
    else
        input.open(QFile::ReadOnly);
    bytes=input.readAll();
    if(bytes.length()==0)
        exit(1);

    pushFilename(file);
    load(QString(bytes));
    popFilename();
}

//Loads a filename for disassembly.
void GoodASM::loadBinFile(QString file){
    QByteArray bytes;

    QFile input(file);
    if(file=="-")
        input.open(stdin, QIODevice::ReadOnly);
    else
        input.open(QFile::ReadOnly);
    bytes=input.readAll();
    if(bytes.length()==0)
        exit(1);

    load(bytes);
}
//Same, but for damage.
void GoodASM::loadDamageFile(QString file){
    QByteArray bytes;

    QFile input(file);
    if(file=="-")
        input.open(stdin, QIODevice::ReadOnly);
    else
        input.open(QFile::ReadOnly);
    bytes=input.readAll();
    if(bytes.length()==0)
        exit(1);

    loadDamage(bytes);
}


//Pushes the filename before including another.
void GoodASM::pushFilename(QString name){
    filenames.push(filename);
    filenamelines.push(line);
    filename=name;
    line=0;
}
//Pops the filename after including another.
QString GoodASM::popFilename(){
    assert(filenames.count()>0);
    assert(filenamelines.count()>0);

    //As an exception, we keep the last filename after popping it.
    QString newfn=filenames.pop();
    if(newfn.length()>0)
        filename=newfn;

    line=filenamelines.pop();
    return filename;
}

// Insert the next instruction.
void GoodASM::append(GAInstruction ins){
    ins.adr=workingadr;
    instructions.append(ins);
    if(type==ASSEMBLY){
        //Only append code if it's not already there.
        bytes.append(ins.code());
        address(); //recalculate working address.
    }else{
        workingadr+=ins.len;
    }
}

// Grab an instruction from an address.
GAInstruction GoodASM::at(uint64_t adr){
    GAInstruction ins=lang->dis(adr);
    return ins;
}

// Confusing when epsilon!=1.
uint8_t GoodASM::byteAt(uint64_t adr){
    //qDebug()<<"Adr="<<adr<<"len="<<bytes.length();

    /* On architectures like PIC16C5x, we swap the bytes
     * as we load them so that our code more closely matches
     * the datasheet.
     */
    if(lang->swapwordonload)
        adr^=1;

    //Important to adjust for the base address.
    adr-=baseaddress;

    if(adr<bytes.length())
        return bytes[(int) adr];

    //Too loud even for verbose mode.  Uncomment if you really need this.
    //if(verbose) qDebug()<<"Illegal fetch from adr"<<adr;

    return 0;
}

// Same but for damage instead of data.
uint8_t GoodASM::damageAt(uint64_t adr){
    /* On architectures like PIC16C5x, we swap the bytes
     * as we load them so that our code more closely matches
     * the datasheet.
     */
    if(lang->swapwordonload)
        adr^=1;

    //Important to adjust for the base address.
    adr-=baseaddress;

    if(adr<damage.length())
        return damage[(int) adr];

    return 0;
}

//Calculated current working address.
uint64_t GoodASM::address(){
    workingadr=baseaddress+bytes.length();
    return workingadr;
}

//Identifies languages for a binary.
QVector<GAGraderGrade> GoodASM::identify(){
    /* This function works by reconfiguring the disassembler
     * into each language and grader, then testing to see if
     * the binary is a match.
     */

    QVector<GAGraderGrade> matches;

    setLanguage(""); //Ensure list is populated.
    setGrader("");   //Ensure list is populated.

    //Collect the scores of each grader.
    foreach(auto g, graders){
        //For all languages.
        if(g->stable)
            matches.append(g->scores(this));
    }

    return matches;
}

//Grades in the current language.
QVector<GAGraderGrade> GoodASM::grade(){
    QVector<GAGraderGrade> matches;
    setGrader("");   //Ensure list is populated.

    //Collect the scores of each grader for the one language.
    foreach(auto g, graders){
        if(g->stable && g->isCompatible(lang))
            matches.append(g->scores(this));
    }

    return matches;
}
