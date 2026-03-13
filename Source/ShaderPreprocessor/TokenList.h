#pragma once
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>


struct DUI;
class Macro;

using TokenString = std::string;

/**
 * Location in source code
 */
struct Location {
    Location() = default;
    Location(unsigned int fileIndex, unsigned int line, unsigned int col)
        : fileIndex(fileIndex)
        , line(line)
        , col(col)
    {}

    Location(const Location &loc) = default;
    Location &operator=(const Location &other) = default;

    /** increment this location by string */
    void adjust(const std::string &str);

    bool operator<(const Location &rhs) const {
        if (fileIndex != rhs.fileIndex)
            return fileIndex < rhs.fileIndex;
        if (line != rhs.line)
            return line < rhs.line;
        return col < rhs.col;
    }

    bool sameline(const Location &other) const {
        return fileIndex == other.fileIndex && line == other.line;
    }

    unsigned int fileIndex{};
    unsigned int line{};
    unsigned int col{};
};

/**
 * token class.
 * @todo don't use std::string representation - for both memory and performance reasons
 */
class Token {
public:
    Token(const TokenString &s, const Location &loc, bool wsahead = false) :
        whitespaceahead(wsahead), location(loc), string(s) {
        flags();
    }

    Token(const Token &tok) :
        macro(tok.macro), op(tok.op), comment(tok.comment), name(tok.name), number(tok.number), whitespaceahead(tok.whitespaceahead), location(tok.location), string(tok.string), mExpandedFrom(tok.mExpandedFrom) {}

    Token &operator=(const Token &tok) = delete;

    const TokenString& str() const {
        return string;
    }
    void setstr(const std::string &s) {
        string = s;
        flags();
    }

    bool isOneOf(const char ops[]) const;
    bool startsWithOneOf(const char c[]) const;
    bool endsWithOneOf(const char c[]) const;
    static bool isNumberLike(const std::string& str) {
        return std::isdigit(static_cast<unsigned char>(str[0])) ||
               (str.size() > 1U && (str[0] == '-' || str[0] == '+') && std::isdigit(static_cast<unsigned char>(str[1])));
    }

    TokenString macro;
    char op;
    bool comment;
    bool name;
    bool number;
    bool whitespaceahead;
    Location location;
    Token *previous{};
    Token *next{};
    mutable const Token *nextcond{};

    const Token *previousSkipComments() const {
        const Token *tok = this->previous;
        while (tok && tok->comment)
            tok = tok->previous;
        return tok;
    }

    const Token *nextSkipComments() const {
        const Token *tok = this->next;
        while (tok && tok->comment)
            tok = tok->next;
        return tok;
    }

    void setExpandedFrom(const Token *tok, const Macro* m) {
        mExpandedFrom = tok->mExpandedFrom;
        mExpandedFrom.insert(m);
        if (tok->whitespaceahead)
            whitespaceahead = true;
    }
    bool isExpandedFrom(const Macro* m) const {
        return mExpandedFrom.find(m) != mExpandedFrom.end();
    }

    void printAll() const;
    void printOut() const;
private:
    void flags() {
        name = (std::isalpha(static_cast<unsigned char>(string[0])) || string[0] == '_' || string[0] == '$')
               && (std::memchr(string.c_str(), '\'', string.size()) == nullptr);
        comment = string.size() > 1U && string[0] == '/' && (string[1] == '/' || string[1] == '*');
        number = isNumberLike(string);
        op = (string.size() == 1U && !name && !comment && !number) ? string[0] : '\0';
    }

    TokenString string;

    std::set<const Macro*> mExpandedFrom;
};

/** Output from preprocessor */
struct Output {
    enum Type : std::uint8_t {
        ERROR, /* #error */
        WARNING, /* #warning */
        MISSING_HEADER,
        INCLUDE_NESTED_TOO_DEEPLY,
        SYNTAX_ERROR,
        PORTABILITY_BACKSLASH,
        UNHANDLED_CHAR_ERROR,
        EXPLICIT_INCLUDE_NOT_FOUND,
        FILE_NOT_FOUND,
        DUI_ERROR
    } type;
    Output(Type type, const Location& loc, std::string msg) : type(type), location(loc), msg(std::move(msg)) {}
    Location location;
    std::string msg;
};

using OutputList = std::list<Output>;

class TokenList {
public:
    class Stream;

    explicit TokenList(std::vector<std::string> &filenames);
    /** generates a token list from the given std::istream parameter */
    TokenList(std::istream &istr, std::vector<std::string> &filenames, const std::string &filename=std::string(), OutputList *outputList = nullptr);
    /** generates a token list from the given buffer */
    template<size_t size>
    TokenList(const char (&data)[size], std::vector<std::string> &filenames, const std::string &filename=std::string(), OutputList *outputList = nullptr)
        : TokenList(reinterpret_cast<const unsigned char*>(data), size-1, filenames, filename, outputList, 0)
    {}
    /** generates a token list from the given buffer */
    template<size_t size>
    TokenList(const unsigned char (&data)[size], std::vector<std::string> &filenames, const std::string &filename=std::string(), OutputList *outputList = nullptr)
        : TokenList(data, size-1, filenames, filename, outputList, 0)
    {}
#if SIMPLECPP_TOKENLIST_ALLOW_PTR
    /** generates a token list from the given buffer */
    TokenList(const unsigned char* data, std::size_t size, std::vector<std::string> &filenames, const std::string &filename=std::string(), OutputList *outputList = nullptr)
        : TokenList(data, size, filenames, filename, outputList, 0)
    {}
    /** generates a token list from the given buffer */
    TokenList(const char* data, std::size_t size, std::vector<std::string> &filenames, const std::string &filename=std::string(), OutputList *outputList = nullptr)
        : TokenList(reinterpret_cast<const unsigned char*>(data), size, filenames, filename, outputList, 0)
    {}
#endif // SIMPLECPP_TOKENLIST_ALLOW_PTR
    /** generates a token list from the given buffer */
    TokenList(std::string_view data, std::vector<std::string> &filenames, const std::string &filename=std::string(), OutputList *outputList = nullptr)
        : TokenList(reinterpret_cast<const unsigned char*>(data.data()), data.size(), filenames, filename, outputList, 0)
    {}
#ifdef __cpp_lib_span
    /** generates a token list from the given buffer */
    TokenList(std::span<const char> data, std::vector<std::string> &filenames, const std::string &filename=std::string(), OutputList *outputList = nullptr)
        : TokenList(reinterpret_cast<const unsigned char*>(data.data()), data.size(), filenames, filename, outputList, 0)
    {}

    /** generates a token list from the given buffer */
    TokenList(std::span<const unsigned char> data, std::vector<std::string> &filenames, const std::string &filename=std::string(), OutputList *outputList = nullptr)
        : TokenList(data.data(), data.size(), filenames, filename, outputList, 0)
    {}
#endif // __cpp_lib_span

    /** generates a token list from the given filename parameter */
    TokenList(const std::string &filename, std::vector<std::string> &filenames, OutputList *outputList = nullptr);
    TokenList(const TokenList &other);
    TokenList(TokenList &&other);
    ~TokenList();
    TokenList &operator=(const TokenList &other);
    TokenList &operator=(TokenList &&other);

    void clear();
    bool empty() const {
        return !frontToken;
    }
    void push_back(Token *tok);

    void dump(bool linenrs = false) const;
    void stringify(std::stringstream& ss, bool linenrs = false) const;

    void readfile(Stream &stream, const std::string &filename=std::string(), OutputList *outputList = nullptr);
    /**
     * @throws std::overflow_error thrown on overflow or division by zero
     * @throws std::runtime_error thrown on invalid expressions
     */
    void constFold();

    void removeComments();

    Token *front() {
        return frontToken;
    }

    const Token *cfront() const {
        return frontToken;
    }

    Token *back() {
        return backToken;
    }

    const Token *cback() const {
        return backToken;
    }

    void deleteToken(Token *tok) {
        if (!tok)
            return;
        Token * const prev = tok->previous;
        Token * const next = tok->next;
        if (prev)
            prev->next = next;
        if (next)
            next->previous = prev;
        if (frontToken == tok)
            frontToken = next;
        if (backToken == tok)
            backToken = prev;
        delete tok;
    }

    void takeTokens(TokenList &other) {
        if (!other.frontToken)
            return;
        if (!frontToken) {
            frontToken = other.frontToken;
        } else {
            backToken->next = other.frontToken;
            other.frontToken->previous = backToken;
        }
        backToken = other.backToken;
        other.frontToken = other.backToken = nullptr;
    }

    /** sizeof(T) */
    std::map<std::string, std::size_t> sizeOfType;

    const std::vector<std::string>& getFiles() const {
        return files;
    }

    const std::string& file(const Location& loc) const;

private:
    TokenList(const unsigned char* data, std::size_t size, std::vector<std::string> &filenames, const std::string &filename, OutputList *outputList, int unused);

    void combineOperators();

    void constFoldUnaryNotPosNeg(Token *tok);
    /**
     * @throws std::overflow_error thrown on overflow or division by zero
     */
    void constFoldMulDivRem(Token *tok);
    void constFoldAddSub(Token *tok);
    void constFoldShift(Token *tok);
    void constFoldComparison(Token *tok);
    void constFoldBitwise(Token *tok);
    void constFoldLogicalOp(Token *tok);
    /**
     * @throws std::runtime_error thrown on invalid expressions
     */
    void constFoldQuestionOp(Token *&tok1);

    std::string readUntil(Stream &stream, const Location &location, char start, char end, OutputList *outputList);
    void lineDirective(unsigned int fileIndex, unsigned int line, Location &location);

    const Token* lastLineTok(int maxsize=1000) const;
    const Token* isLastLinePreprocessor(int maxsize=1000) const;

    unsigned int fileIndex(const std::string &filename);

    Token *frontToken;
    Token *backToken;
    std::vector<std::string> &files;
};


/** Tracking how macros are used */
struct MacroUsage {
    explicit MacroUsage(bool macroValueKnown_) : macroValueKnown(macroValueKnown_) {}
    std::string macroName;
    Location macroLocation;
    Location useLocation;
    bool macroValueKnown;
};

/** Tracking #if/#elif expressions */
struct IfCond {
    explicit IfCond(const Location& location, const std::string &E, long long result) : location(location), E(E), result(result) {}
    Location location; // location of #if/#elif
    std::string E; // preprocessed condition
    long long result; // condition result
};

struct FileData {
    /** The canonical filename associated with this data */
    std::string filename;
    /** The tokens associated with this file */
    TokenList tokens;
};

class FileDataCache {
public:
    FileDataCache() = default;

    FileDataCache(const FileDataCache &) = delete;
    FileDataCache(FileDataCache &&) = default;

    FileDataCache &operator=(const FileDataCache &) = delete;
    FileDataCache &operator=(FileDataCache &&) = default;

    /** Get the cached data for a file, or load and then return it if it isn't cached.
     *  returns the file data and true if the file was loaded, false if it was cached. */
    std::pair<FileData *, bool> get(const std::string &sourcefile, const std::string &header, const DUI &dui, bool systemheader, std::vector<std::string> &filenames, OutputList *outputList);

    void insert(FileData data) {
        // NOLINTNEXTLINE(misc-const-correctness) - FP
        auto *const newdata = new FileData(std::move(data));

        mData.emplace_back(newdata);
        mNameMap.emplace(newdata->filename, newdata);
    }

    void clear() {
        mNameMap.clear();
        mIdMap.clear();
        mData.clear();
    }

    using container_type = std::vector<std::unique_ptr<FileData>>;
    using iterator = container_type::iterator;
    using const_iterator = container_type::const_iterator;
    using size_type = container_type::size_type;

    size_type size() const {
        return mData.size();
    }
    iterator begin() {
        return mData.begin();
    }
    iterator end() {
        return mData.end();
    }
    const_iterator begin() const {
        return mData.begin();
    }
    const_iterator end() const {
        return mData.end();
    }
    const_iterator cbegin() const {
        return mData.cbegin();
    }
    const_iterator cend() const {
        return mData.cend();
    }

private:
    struct FileID {
        struct {
            std::uint64_t VolumeSerialNumber;
            struct {
                std::uint64_t IdentifierHi;
                std::uint64_t IdentifierLo;
            } FileId;
        } fileIdInfo;

        bool operator==(const FileID &that) const noexcept {
            return fileIdInfo.VolumeSerialNumber == that.fileIdInfo.VolumeSerialNumber &&
                   fileIdInfo.FileId.IdentifierHi == that.fileIdInfo.FileId.IdentifierHi &&
                   fileIdInfo.FileId.IdentifierLo == that.fileIdInfo.FileId.IdentifierLo;
        }

        struct Hasher {
            std::size_t operator()(const FileID &id) const {
                return static_cast<std::size_t>(id.fileIdInfo.FileId.IdentifierHi ^ id.fileIdInfo.FileId.IdentifierLo ^
                                                id.fileIdInfo.VolumeSerialNumber);
            }
        };
    };

    using name_map_type = std::unordered_map<std::string, FileData *>;
    using id_map_type = std::unordered_map<FileID, FileData *, FileID::Hasher>;

    static bool getFileId(const std::string &path, FileID &id);

    std::pair<FileData *, bool> tryload(name_map_type::iterator &name_it, const DUI &dui, std::vector<std::string> &filenames, OutputList *outputList);

    container_type mData;
    name_map_type mNameMap;
    id_map_type mIdMap;
};

/** Converts character literal (including prefix, but not ud-suffix) to long long value.
 *
 * Assumes ASCII-compatible single-byte encoded str for narrow literals
 * and UTF-8 otherwise.
 *
 * For target assumes
 * - execution character set encoding matching str
 * - UTF-32 execution wide-character set encoding
 * - requirements for __STDC_UTF_16__, __STDC_UTF_32__ and __STDC_ISO_10646__ satisfied
 * - char16_t is 16bit wide
 * - char32_t is 32bit wide
 * - wchar_t is 32bit wide and unsigned
 * - matching char signedness to host
 * - matching sizeof(int) to host
 *
 * For host assumes
 * - ASCII-compatible execution character set
 *
 * For host and target assumes
 * - CHAR_BIT == 8
 * - two's complement
 *
 * Implements multi-character narrow literals according to GCC's behavior,
 * except multi code unit universal character names are not supported.
 * Multi-character wide literals are not supported.
 * Limited support of universal character names for non-UTF-8 execution character set encodings.
 * @throws std::runtime_error thrown on invalid literal
 */
long long characterLiteralToLL(const std::string& str);

FileDataCache load(const TokenList &rawtokens, std::vector<std::string> &filenames, const DUI &dui, OutputList *outputList = nullptr, FileDataCache cache = {});

/**
 * Preprocess
 * @todo simplify interface
 * @param output TokenList that receives the preprocessing output
 * @param rawtokens Raw tokenlist for top sourcefile
 * @param files internal data of simplecpp
 * @param cache output from load()
 * @param dui defines, undefs, and include paths
 * @param outputList output: list that will receive output messages
 * @param macroUsage output: macro usage
 * @param ifCond output: #if/#elif expressions
 */
void preprocess(TokenList &output, const TokenList &rawtokens, std::vector<std::string> &files, FileDataCache &cache, const DUI &dui, OutputList *outputList = nullptr, std::list<MacroUsage> *macroUsage = nullptr, std::list<IfCond> *ifCond = nullptr);

/**
 * Deallocate data
 */
void cleanup(FileDataCache &cache);

/** Simplify path */
std::string simplifyPath(std::string path);

/** Convert Cygwin path to Windows path */
std::string convertCygwinToWindowsPath(const std::string &cygwinPath);

/** Checks if given path is absolute */
bool isAbsolutePath(const std::string &path);