#include "galanguage.h"
#include "gaparameter.h"
#include "gamnemonic.h"

class GALangRISCV : public GALanguage
{
public:
    GALangRISCV();
};

class GAParameterRiscvReg : public GAParameter {
    public:
        GAParameterRiscvReg(const char* mask);
        int match(GAParserOperand *op, int len) override;
    
        QString decode(GALanguage *lang, uint64_t adr,
                       const char *bytes, int inslen) override;
    
        void encode(GALanguage *lang,
                    uint64_t adr, QByteArray &bytes,
                    GAParserOperand op,
                    int inslen
                    ) override;
};


class GAParameterRiscvImm12 : public GAParameter {
public:
    GAParameterRiscvImm12(const char* mask);
    int match(GAParserOperand *op, int len) override;

    QString decode(GALanguage *lang, uint64_t adr,
                   const char *bytes, int inslen) override;
    void encode(GALanguage *lang,
                uint64_t adr, QByteArray &bytes,
                GAParserOperand op,
                int inslen
                ) override;
};

class GAParameterRiscvUtypeImm20 : public GAParameter {
public:
    GAParameterRiscvUtypeImm20(const char* mask);
    int match(GAParserOperand *op, int len) override;

    QString decode(GALanguage *lang, uint64_t adr,
                   const char *bytes, int inslen) override;

    void encode(GALanguage *lang,
                uint64_t adr, QByteArray &bytes,
                GAParserOperand op,
                int inslen
                ) override;
};

class GAParameterRiscvBtypeImm13 : public GAParameter {
public:
    GAParameterRiscvBtypeImm13(const char* mask);
    int match(GAParserOperand *op, int len) override;

    QString decode(GALanguage *lang, uint64_t adr,
                   const char *bytes, int inslen) override;

    void encode(GALanguage *lang,
                uint64_t adr, QByteArray &bytes,
                GAParserOperand op,
                int inslen
                ) override;
};

class GAParameterRiscvJtypeImm21 : public GAParameter {
public:
    GAParameterRiscvJtypeImm21(const char* mask);
    int match(GAParserOperand *op, int len) override;

    QString decode(GALanguage *lang, uint64_t adr,
                   const char *bytes, int inslen) override;

    void encode(GALanguage *lang,
                uint64_t adr, QByteArray &bytes,
                GAParserOperand op,
                int inslen
                ) override;
};

class GAParameterRiscvStypeImm12 : public GAParameter {
public:
    GAParameterRiscvStypeImm12(const char* mask);
    int match(GAParserOperand *op, int len) override;

    QString decode(GALanguage *lang, uint64_t adr,
                   const char *bytes, int inslen) override;

    void encode(GALanguage *lang,
                uint64_t adr, QByteArray &bytes,
                GAParserOperand op,
                int inslen
                ) override;
};

class GAParameterRiscvFencePredSucc : public GAParameter {
public:
    GAParameterRiscvFencePredSucc(const char* mask);
    int match(GAParserOperand *op, int len) override;

    QString decode(GALanguage *lang, uint64_t adr,
                   const char *bytes, int inslen) override;

    void encode(GALanguage *lang,
                uint64_t adr, QByteArray &bytes,
                GAParserOperand op,
                int inslen
                ) override;
};

class GAParameterRiscvShamt5 : public GAParameter {
public:
    GAParameterRiscvShamt5(const char* mask);
    int match(GAParserOperand *op, int len) override;

    QString decode(GALanguage *lang, uint64_t adr,
                   const char *bytes, int inslen) override;

    void encode(GALanguage *lang,
                uint64_t adr, QByteArray &bytes,
                GAParserOperand op,
                int inslen
                ) override;
};

class GAParameterRiscvCsr12 : public GAParameter {
public:
    GAParameterRiscvCsr12(const char* mask);
    int match(GAParserOperand *op, int len) override;

    QString decode(GALanguage *lang, uint64_t adr,
                   const char *bytes, int inslen) override;

    void encode(GALanguage *lang,
                uint64_t adr, QByteArray &bytes,
                GAParserOperand op,
                int inslen
                ) override;
};

class GAParameterRiscvUimm5 : public GAParameter {
public:
    GAParameterRiscvUimm5(const char* mask);
    int match(GAParserOperand *op, int len) override;

    QString decode(GALanguage *lang, uint64_t adr,
                   const char *bytes, int inslen) override;

    void encode(GALanguage *lang,
                uint64_t adr, QByteArray &bytes,
                GAParserOperand op,
                int inslen
                ) override;
};

// C Extension parameter types for compressed 16-bit instructions

class GAParameterRiscvCompReg : public GAParameter {
public:
    GAParameterRiscvCompReg(const char* mask);
    int match(GAParserOperand *op, int len) override;

    QString decode(GALanguage *lang, uint64_t adr,
                   const char *bytes, int inslen) override;

    void encode(GALanguage *lang,
                uint64_t adr, QByteArray &bytes,
                GAParserOperand op,
                int inslen
                ) override;
};

class GAParameterRiscvCIimm6 : public GAParameter {
public:
    GAParameterRiscvCIimm6(const char* mask);
    int match(GAParserOperand *op, int len) override;

    QString decode(GALanguage *lang, uint64_t adr,
                   const char *bytes, int inslen) override;

    void encode(GALanguage *lang,
                uint64_t adr, QByteArray &bytes,
                GAParserOperand op,
                int inslen
                ) override;
};

class GAParameterRiscvCIUimm6 : public GAParameter {
public:
    GAParameterRiscvCIUimm6(const char* mask);
    int match(GAParserOperand *op, int len) override;

    QString decode(GALanguage *lang, uint64_t adr,
                   const char *bytes, int inslen) override;

    void encode(GALanguage *lang,
                uint64_t adr, QByteArray &bytes,
                GAParserOperand op,
                int inslen
                ) override;
};

class GAParameterRiscvCIUimm6Srai : public GAParameter {
public:
    GAParameterRiscvCIUimm6Srai(const char* mask);
    int match(GAParserOperand *op, int len) override;

    QString decode(GALanguage *lang, uint64_t adr,
                   const char *bytes, int inslen) override;

    void encode(GALanguage *lang,
                uint64_t adr, QByteArray &bytes,
                GAParserOperand op,
                int inslen
                ) override;
};

class GAParameterRiscvCIimm7 : public GAParameter {
public:
    GAParameterRiscvCIimm7(const char* mask);
    int match(GAParserOperand *op, int len) override;

    QString decode(GALanguage *lang, uint64_t adr,
                   const char *bytes, int inslen) override;

    void encode(GALanguage *lang,
                uint64_t adr, QByteArray &bytes,
                GAParserOperand op,
                int inslen
                ) override;
};

class GAParameterRiscvCIWimm9 : public GAParameter {
public:
    GAParameterRiscvCIWimm9(const char* mask);
    int match(GAParserOperand *op, int len) override;

    QString decode(GALanguage *lang, uint64_t adr,
                   const char *bytes, int inslen) override;

    void encode(GALanguage *lang,
                uint64_t adr, QByteArray &bytes,
                GAParserOperand op,
                int inslen
                ) override;
};

class GAParameterRiscvCLimm5 : public GAParameter {
public:
    GAParameterRiscvCLimm5(const char* mask);
    int match(GAParserOperand *op, int len) override;

    QString decode(GALanguage *lang, uint64_t adr,
                   const char *bytes, int inslen) override;

    void encode(GALanguage *lang,
                uint64_t adr, QByteArray &bytes,
                GAParserOperand op,
                int inslen
                ) override;
};

class GAParameterRiscvCSimm5 : public GAParameter {
public:
    GAParameterRiscvCSimm5(const char* mask);
    int match(GAParserOperand *op, int len) override;

    QString decode(GALanguage *lang, uint64_t adr,
                   const char *bytes, int inslen) override;

    void encode(GALanguage *lang,
                uint64_t adr, QByteArray &bytes,
                GAParserOperand op,
                int inslen
                ) override;
};

class GAParameterRiscvCSSimm7 : public GAParameter {
public:
    GAParameterRiscvCSSimm7(const char* mask);
    int match(GAParserOperand *op, int len) override;

    QString decode(GALanguage *lang, uint64_t adr,
                   const char *bytes, int inslen) override;

    void encode(GALanguage *lang,
                uint64_t adr, QByteArray &bytes,
                GAParserOperand op,
                int inslen
                ) override;
};

class GAParameterRiscvCBimm9 : public GAParameter {
public:
    GAParameterRiscvCBimm9(const char* mask);
    int match(GAParserOperand *op, int len) override;

    QString decode(GALanguage *lang, uint64_t adr,
                   const char *bytes, int inslen) override;

    void encode(GALanguage *lang,
                uint64_t adr, QByteArray &bytes,
                GAParserOperand op,
                int inslen
                ) override;
};

class GAParameterRiscvCJimm12 : public GAParameter {
public:
    GAParameterRiscvCJimm12(const char* mask);
    int match(GAParserOperand *op, int len) override;

    QString decode(GALanguage *lang, uint64_t adr,
                   const char *bytes, int inslen) override;

    void encode(GALanguage *lang,
                uint64_t adr, QByteArray &bytes,
                GAParserOperand op,
                int inslen
                ) override;
};

class GAParameterRiscvCBimm6Andi : public GAParameter {
public:
    GAParameterRiscvCBimm6Andi(const char* mask);
    int match(GAParserOperand *op, int len) override;

    QString decode(GALanguage *lang, uint64_t adr,
                   const char *bytes, int inslen) override;

    void encode(GALanguage *lang,
                uint64_t adr, QByteArray &bytes,
                GAParserOperand op,
                int inslen
                ) override;
};