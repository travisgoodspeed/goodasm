#include <QCoreApplication>
#include <QCommandLineParser>
#include <QByteArray>
#include <QFile>
#include <QDebug>
#include <iostream>

#include "gaparser.h"
#include "goodasm.h"
#include "gareplxx.h"

int main(int argc, char *argv[]){
    int errorcount=0;

    //Empty language sees everything as bytes in disassembly.
    QString language="";
    GALanguage *lang=0;
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("GoodASM is an easily retargetable assembler for CISC microcontrollers.");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("input", "ROM image to decode or source to assemble.");

    /* Here we have a bunch of arguments.  We try to roughly group them by purpose,
     * but the list will soon get messy.
     */

    QCommandLineOption verboseOption(QStringList()<<"V"<<"verbose",
                                     "Talk too much."
                                     );
    parser.addOption(verboseOption);

    QCommandLineOption cheatOption(QStringList()<<"cheat",
                                   "Print a cheat sheet of mnemonics."
                                   );
    parser.addOption(cheatOption);

    QCommandLineOption opcodetableOption(QStringList()<<"T"<<"opcodetable",
                                   "Print a table of opcodes."
                                   );
    parser.addOption(opcodetableOption);

    QCommandLineOption testOption(QStringList()<<"t"<<"test",
                                  "Selftest the selected language."
                                  );
    parser.addOption(testOption);
    QCommandLineOption fuzzOption(QStringList()<<"fuzz",
                                  "Fuzz test to find crashes."
                                  );
    parser.addOption(fuzzOption);

    QCommandLineOption gradeOption(QStringList()<<"g"<<"grade",
                                   "Is this binary in the chosen language?"
                                   );
    parser.addOption(gradeOption);

    QCommandLineOption identOption(QStringList()<<"i"<<"identify",
                                   "In what language is this binary?"
                                   );
    parser.addOption(identOption);

    QCommandLineOption disOption(QStringList()<<"d"<<"dis",
                                 "Disassemble a binary input."
                                 );
    parser.addOption(disOption);




    QCommandLineOption baseOption(QStringList()<<"base",
                                 "Base address of an input binary.",
                                 "base"
                                 );
    parser.addOption(baseOption);

    QCommandLineOption outOption(QStringList()<<"o"<<"export",
                                 "Output filename.",
                                 "file"
                                 );
    parser.addOption(outOption);

    QCommandLineOption symboltableOption(QStringList()<<"S"<<"symboltable",
                                 "Export symbol table",
                                 "file"
                                 );
    parser.addOption(symboltableOption);


    QCommandLineOption replOption(QStringList()<<"repl",
                                  "Interactive shell."
                                  );
    parser.addOption(replOption);

    QCommandLineOption waitOption(QStringList()<<"wait",
                                  "Wait after cleanup."
                                  );
    parser.addOption(waitOption);


    QCommandLineOption listOption(QStringList()<<"L"<<"list",
                                  "Print a listing to stdout."
                                  );
    parser.addOption(listOption);

    QCommandLineOption hexOption(QStringList()<<"H"<<"hex",
                                 "Print hex of output to stdout."
                                 );
    parser.addOption(hexOption);

    QCommandLineOption nasmOption(QStringList()<<"N"<<"nasm",
                                  "Output is legal in NASM."
                                  );
    parser.addOption(nasmOption);

    QCommandLineOption goodasmOption(QStringList()<<"F"<<"goodasm",
                                  "Output is legal in GoodASM."
                                  );
    parser.addOption(goodasmOption);

    QCommandLineOption cOption(QStringList()<<"C",
                                  "Output is legal in C."
                                  );
    parser.addOption(cOption);

    QCommandLineOption goOption(QStringList()<<"G"<<"golang",
                               "Output is legal in Golang."
                               );
    parser.addOption(goOption);

    QCommandLineOption yaraxOption(QStringList()<<"Y"<<"yara-x",
                                   "Output is legal in Yara-X."
                                   );
    parser.addOption(yaraxOption);
    QCommandLineOption markdownOption(QStringList()<<"M"<<"markdown",
                                   "Output is legal in Markdown."
                                   );
    parser.addOption(markdownOption);

    QCommandLineOption listbytesOption(QStringList()<<"b"<<"bytes",
                                       "List bytes in output."
                                       );
    parser.addOption(listbytesOption);

    QCommandLineOption listbitsOption(QStringList()<<"B"<<"bits",
                                      "List bits in output."
                                      );
    parser.addOption(listbitsOption);
    QCommandLineOption listdbitsOption(QStringList()<<"D"<<"dbits",
                                       "List damage bits in output."
                                       );
    parser.addOption(listdbitsOption);
    QCommandLineOption damageOption(QStringList()<<"damage",
                                 "Damage filename.",
                                 "mask.bin"
                                 );
    parser.addOption(damageOption);

    QCommandLineOption autocommentOption(QStringList()<<"A"<<"auto",
                                         "Comment with cheat sheet when available."
                                         );
    parser.addOption(autocommentOption);


    QCommandLineOption listadrOption(QStringList()<<"a"<<"adr",
                                     "List addresses in output."
                                     );
    parser.addOption(listadrOption);

    /* Here are the languages that we support.  The help message should give an idea
     * of the language's stability and quality.
     *
     * FIXME: Autogenerate the language options.
     */
    QCommandLineOption lang8086(QStringList()<<"8086",
                                "Intel 8086 (broken)"
                                );
    parser.addOption(lang8086);
    QCommandLineOption lang6502(QStringList()<<"6502",
                                "MOS 6502 (stable)"
                                );
    parser.addOption(lang6502);
    QCommandLineOption lang6805(QStringList()<<"6805",
                                "68HC05 (stable)"
                                );
    parser.addOption(lang6805);
    QCommandLineOption langST7(QStringList()<<"st7",
                                "ST7 (alpha)"
                                );
    parser.addOption(langST7);
    QCommandLineOption langFcard(QStringList()<<"fcard",
                                "68HC05SC21 DSS P1 F-Card (stable)"
                                );
    parser.addOption(langFcard);
    QCommandLineOption langSM83(QStringList()<<"sm83"<<"gb",
                                "Sharp SM83 / Gameboy (stable)"
                                );
    parser.addOption(langSM83);
    QCommandLineOption langZ80(QStringList()<<"z80",
                               "Zilog Z80 (stable)"
                               );
    parser.addOption(langZ80);
    QCommandLineOption langZ8(QStringList()<<"z8",
                               "Zilog Z8 (beta)"
                               );
    parser.addOption(langZ8);
    QCommandLineOption langTLCS47(QStringList()<<"tlcs47",
                                  "Toshiba TLCS47 (alpha)"
                                  );
    parser.addOption(langTLCS47);
    QCommandLineOption lang8051(QStringList()<<"8051",
                                "Intel 8051 (alpha)"
                                );
    parser.addOption(lang8051);
    QCommandLineOption langUCOM43(QStringList()<<"ucom43",
                                  "Sharp UCOM43 (alpha)"
                                  );
    parser.addOption(langUCOM43);
    QCommandLineOption langS2000(QStringList()<<"s2000"<<"emz1001",
                                 "AMI S2000 / Iskra EMZ1001 (alpha)"
                                 );
    parser.addOption(langS2000);
    QCommandLineOption langPIC16C5x(QStringList()<<"pic16c5x",
                                    "PIC16C5x (12-bit) (alpha)"
                                    );
    parser.addOption(langPIC16C5x);
    QCommandLineOption langMARC4(QStringList()<<"marc4",
                                 "Atmel MARC4 (alpha)"
                                 );
    parser.addOption(langMARC4);
    QCommandLineOption langChip8(QStringList()<<"chip8",
                                 "Chip-8 (broken)"
                                 );
    parser.addOption(langChip8);
    QCommandLineOption lang8080(QStringList()<<"8080",
                               "Intel 8080 (alpha)"
                               );
    parser.addOption(lang8080);
    QCommandLineOption langH83(QStringList()<<"h83",
                                 "Renesas H83 (alpha)"
                                 );
    parser.addOption(langH83);
    QCommandLineOption langARM7TDMI(QStringList()<<"arm7tdmi",
                               "ARM7TDMI (broken)"
                               );
    parser.addOption(langARM7TDMI);
    QCommandLineOption langTMS320C28x(QStringList()<<"tms320c28x",
                                 "TI TMS320C28x (broken)"
                                 );
    parser.addOption(langTMS320C28x);
    QCommandLineOption langTI80(QStringList()<<"ti80",
                                      "TI 80 (broken)"
                                      );
    parser.addOption(langTI80);



    if(argc==1)
        std::cout<<parser.helpText().toStdString();

    parser.process(a);


    //FIXME: Manage the languages from a list.
    if(parser.isSet(lang8086))
        language="8086";
    else if(parser.isSet(langTLCS47))
        language="tlcs47";
    else if(parser.isSet(langUCOM43))
        language="ucom43";
    else if(parser.isSet(langS2000))
        language="s2000";
    else if(parser.isSet(langPIC16C5x))
        language="pic16c5x";
    else if(parser.isSet(langMARC4))
        language="marc4";
    else if(parser.isSet(lang6502))
        language="6502";
    else if(parser.isSet(lang6805))
        language="6805";
    else if(parser.isSet(langST7))
        language="st7";
    else if(parser.isSet(langFcard))
        language="fcard";
    else if(parser.isSet(lang8051))
        language="8051";
    else if(parser.isSet(langSM83))
        language="sm83";
    else if(parser.isSet(langChip8))
        language="chip8";
    else if(parser.isSet(langZ80))
        language="z80";
    else if(parser.isSet(langZ8))
        language="z8";
    else if(parser.isSet(lang8080))
        language="8080";
    else if(parser.isSet(langH83))
        language="h83";
    else if(parser.isSet(langARM7TDMI))
        language="arm7tdmi";
    else if(parser.isSet(langTMS320C28x))
        language="tms320c28x";
    else if(parser.isSet(langTI80))
        language="ti80";

    //We allocate this dynamically to test that we free properly.
    GoodASM *goodasm=new GoodASM(language);
    lang=goodasm->lang;


    if(parser.isSet(baseOption)){
        bool okay;
        uint64_t base=GAParser::str2uint(parser.value(baseOption), &okay);
        if(!okay){
            qDebug()<<"Can't parse base address: "<<parser.value(baseOption);
            qDebug()<<"Thought it was "<<Qt::hex<<base;
            exit(1);
        }
        goodasm->baseaddress=base;
    }

    //Listing flags.
    bool listing=true;
    if(parser.isSet(nasmOption))
        goodasm->setListing("nasm");
    else if(parser.isSet(cOption))
        goodasm->setListing("c");
    else if(parser.isSet(goOption))
        goodasm->setListing("go");
    else if(parser.isSet(yaraxOption))
        goodasm->setListing("yarax");
    else if(parser.isSet(markdownOption))
        goodasm->setListing("markdown");
    else if(parser.isSet(hexOption))
        goodasm->setListing("hex");
    else if(parser.isSet(goodasmOption))
        goodasm->setListing("goodasm");
    else
        listing=parser.isSet(listOption);

    //Listing modifiers.
    goodasm->listbytes=parser.isSet(listbytesOption)?-1:0;
    if(parser.isSet(listbitsOption)){
        goodasm->listbits=true;
        goodasm->listbytes=-1;
    }
    if(parser.isSet(listdbitsOption)){
        goodasm->listdbits=true;
        goodasm->listbytes=-1;
    }
    goodasm->listadr=parser.isSet(listadrOption)?1:0;
    goodasm->verbose=parser.isSet(verboseOption)?1:0;
    goodasm->autocomment=parser.isSet(autocommentOption);


    if(parser.isSet(cheatOption)){
        std::cout<<lang->cheatSheet().toStdString();
        exit(1);
    }
    if(parser.isSet(opcodetableOption)){
        std::cout<<goodasm->opcodeTable().toStdString();
        exit(0);
    }

    //Selftest returns a code for integration testing.
    if(parser.isSet(testOption)){
        if(goodasm->selftest_all()){
            std::cout<<"Selftest passed.\n";
            return 0;
        }else{
            std::cout<<"Selftest of "<<goodasm->lang->name.toStdString()<<" failed.\n";
            return 1;
        }
    }

    //Fuzz tests to identify crashes.
    if(parser.isSet(fuzzOption) && parser.isSet(identOption)){
        if(!goodasm->selftest_fuzz()){
            std::cout<<"Fuzz test failed.\n";
            return 1;
        }
        goodasm->identify();
        std::cout<<"Identity of fuzzed target did not crash.\n";
        return 0;
    }else if(parser.isSet(fuzzOption)){
        if(goodasm->selftest_fuzz()){
            std::cout<<"Fuzz test passed.\n";
            return 0;
        }else{
            std::cout<<"Selftest of "<<goodasm->lang->name.toStdString()<<" failed.\n";
            return 1;
        }
    }

    //Positional arguments.
    const QStringList args = parser.positionalArguments();


    if(parser.isSet(replOption) || args.count()==0){
        //New interactive mode, without readline.
        return gareplxx_encode(goodasm);
    }else if(args.count()==1 && parser.isSet(disOption)){    //Disassemble a binary.
        if(parser.isSet(damageOption))
            goodasm->loadDamageFile(parser.value(damageOption));
        goodasm->loadBinFile(args[0]);
        std::cout<<goodasm->source().toStdString();
    } else if(args.count()==1
               && !parser.isSet(disOption)
               && !parser.isSet(gradeOption)
               && !parser.isSet(identOption)){    //Assemble from source.
        goodasm->loadFile(args[0]);

        if(listing)
            std::cout<<goodasm->source().toStdString();

        if(parser.isSet(outOption)){
            QFile out(parser.value(outOption));
            if( (args[0]=="-" && out.open(stdout, QIODevice::WriteOnly))
                || out.open(QFile::WriteOnly)){
                out.write(goodasm->code());
                out.close();
            }else{
                qDebug()<<"Error opening "<<parser.value(outOption);
                exit(1);
            }
        }
        if(parser.isSet(symboltableOption)){
            QFile stfile(parser.value(symboltableOption));
            if( (args[0]=="-" && stfile.open(stdout, QIODevice::WriteOnly))
                || stfile.open(QFile::WriteOnly)){
                stfile.write(goodasm->symbols.exportTable().toStdString().c_str());
                stfile.close();
            }else{
                qDebug()<<"Error opening "<<parser.value(symboltableOption);
                exit(1);
            }
        }

        //Sometimes we don't define all symbols.  This is really bad.
        if(!goodasm->symbols.complete()){
            errorcount++;
            qDebug()<<"Undefined symbols:";
            QStringList list=goodasm->symbols.missingSymbols();
            for(int i=0; i<list.count(); i++){
                qDebug()<<"\t"<<list[i];
            }
        }
    } else if(args.count()==1 && parser.isSet(gradeOption)){  //Is this binary in this language?
        goodasm->loadBinFile(args[0]);

        goodasm->setGrader("");   //Ensure list is populated.
        int valids=0, invalids=0;
        foreach(GAGraderGrade g, goodasm->grade()){
            bool valid=g.score>50;
            printf("%s\t%s\n",
                   g.grader->name.toStdString().c_str(),
                   (valid?"valid":"invalid")
                   );
            if(valid) valids++;
            else invalids++;
        }
        if(valids>invalids) return 0;
        else return 1;
    } else if(args.count()==1 && parser.isSet(identOption)){  //In what language is the binary?
        goodasm->loadBinFile(args[0]);
        QVector<GAGraderGrade> grades=goodasm->identify();
        foreach(auto g, grades){
            printf("%s\t%s\t%ld\n",
                   g.lang->name.toStdString().c_str(),
                   g.grader->name.toStdString().c_str(),
                   (long) g.score
                   );
        }
    }

    errorcount=goodasm->printErrors();
    delete goodasm;
    if(parser.isSet(waitOption)){
        std::cout<<"Waiting on ^C.\n";
        while(1);
    }

    return errorcount;
}
