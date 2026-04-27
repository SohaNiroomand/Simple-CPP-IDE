#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <iostream>
#include <vector>
#include <cstring>
#include <unordered_set>
#include <sstream>
#include <stack>
#include <algorithm>
#include <unordered_set>
#include <fstream>
#include <cstdlib> // For system()
#include <regex>

bool compile(const std::string &filename)
{
    std::string compileCommand = "g++ " + filename + " -o output";
    int result = std::system(compileCommand.c_str());
    return (result == 0);
}

void run(const std::string &filename)
{
#ifdef _WIN32
    std::string runCommand = filename + ".exe";
#else
    std::string runCommand = "./" + filename;
#endif
    std::system(runCommand.c_str());
}

//operator error
std::vector<std::string> validOperators = {"+", "-", "=", "*", "%", "+=", "-=", "*=", "/=", "==", ";", "(", ")", "<", ">", "[", "]", "{", "}", ">>", "<<", "#"};

// تابع بررسی عملگرهای نامعتبر
bool isValidOperator(const std::string& op) {
    return std::find(validOperators.begin(), validOperators.end(), op) != validOperators.end();
}
SDL_Texture* darkModeBackground = nullptr;//برای تصویر پس زمینه
SDL_Texture* lightModeBackground = nullptr;

//if else tekrari
std::vector<std::string> detectRepeatedIfElse(const std::vector<std::string>& lines) {
    std::regex ifPattern(R"(\bif\s*\(([^)]+)\))"); // پیدا کردن شرط‌های if
    std::smatch match;

    std::string lastCondition = "";
    bool lastWasIf = false;

    std::vector<std::string> warnings;

    for (int i = 0; i < lines.size(); ++i) {
        std::string line = lines[i];

        if (std::regex_search(line, match, ifPattern)) {
            std::string condition = match[1]; // دریافت شرط داخل پرانتز

            if (lastWasIf && condition == lastCondition) {
                warnings.push_back("Warning: same if at line " + std::to_string(i + 1));
            }

            lastCondition = condition;
            lastWasIf = true;
        }
        else if (line.find("else") != std::string::npos) {
            lastWasIf = false; // اگر else پیدا شد، شرط رو نادیده بگیر
        }
    }

    return warnings;
}


// اندازه پنجره
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int horizontalScrollOffset = 0;
// برای اسکرول
int levenshteinDistance(const std::string& a, const std::string& b) {//برای الگوریتم Levenshtein Distance
    int m = a.size();
    int n = b.size();

    // ایجاد یک ماتریس برای ذخیره فاصله‌ها
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1));

    // پر کردن ماتریس با مقادیر اولیه
    for (int i = 0; i <= m; i++) dp[i][0] = i;
    for (int j = 0; j <= n; j++) dp[0][j] = j;

    // محاسبه فاصله Levenshtein
    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + cost});
        }
    }

    return dp[m][n];
}
void ensureLastLineVisible(int currentLine, int &scrollOffset, int SCREEN_HEIGHT, int LINE_HEIGHT, int totalLines);

std::unordered_set<std::string> definedVariables; //برای متغیرهای تعریف نشده

std::unordered_set<std::string> validKeywords = {
        "int", "float", "double", "char", "if", "else", "while", "for", "return",
        "void", "switch", "case", "break", "continue", "struct", "class", "public",
        "private", "protected", "const", "typedef", "namespace", "using", "try",
        "catch", "throw"
};
// انواع توکن برای رنگ‌آمیزی
enum class TokenType {
    Keyword,
    DataType,
    FunctionName,
    VariableName,
    StringLiteral,
    CharacterLiteral,
    NumberLiteral,
    Comment,
    PreprocessorDirective,
    Operator,
    Parenthesis,
    Unknown
};

struct Token {
    TokenType type;
    int start;
    int length;
};

// توابع کمکی برای join و split
static std::string join(const std::vector<std::string>& lines, char delim) {
    std::string result;
    for (int i = 0; i < (int)lines.size(); i++) {
        result += lines[i];
        if (i < (int)lines.size() - 1) {
            result += delim;
        }
    }
    return result;
}

static std::vector<std::string> split(const std::string& str, char delim) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, delim)) {
        result.push_back(item);
    }
    // اگر کاملاً خالی بود، حداقل یک خط خالی بسازد
    if (str.empty()) {
        result.push_back("");
    }
    return result;
}

// شناسایی نوع توکن (ساده‌سازی‌شده)
TokenType identifyTokenType(const std::string& token) {
    static const std::unordered_set<std::string> keywords = {"class", "if", "while"};
    static const std::unordered_set<std::string> dataTypes = {"int", "float", "double"};

    if (keywords.count(token)) return TokenType::Keyword;
    if (dataTypes.count(token)) return TokenType::DataType;
    if (token.find("#include") == 0) return TokenType::PreprocessorDirective;
    if (token.size() == 3 && token.front() == '\'' && token.back() == '\'') return TokenType::CharacterLiteral;
    if (token == "(" || token == ")" || token == "{" || token == "}" || token == "[" || token == "]") return TokenType::Parenthesis;

    if (!token.empty() && (std::isalpha(token[0]) || token[0] == '_')) {
        // ساده‌سازی: اگر آخرش '(' بود، می‌گوییم تابع است
        if (!token.empty() && token.back() == '(') return TokenType::FunctionName;
        return TokenType::VariableName;
    }

    if (!token.empty() && std::all_of(token.begin(), token.end(), ::isdigit)) {
        return TokenType::NumberLiteral;
    }

    if (token.size() >= 2 && token.front() == '"' && token.back() == '"') {
        return TokenType::StringLiteral;
    }

    if (token.find("//") == 0) {
        return TokenType::Comment;
    }

    return TokenType::Unknown;
}

// توکنیزه کردن یک خط
std::vector<Token> tokenizeLine(const std::string& line) {
    std::vector<Token> tokens;
    int currentIndex = 0;
    bool inString = false;
    int stringStart = 0;

    for (int i = 0; i < (int)line.length(); ++i) {
        if (line[i] == '"') {
            if (inString) {
                tokens.push_back({TokenType::StringLiteral, stringStart, i - stringStart + 1});
                inString = false;
                currentIndex = i + 1;
            } else {
                stringStart = i;
                inString = true;
            }
        }
        else if (inString) {
            // اگر داخل رشته هستیم، فقط منتظر بسته شدن کوتیشن می‌مانیم
            continue;
        }
        else if (std::isspace(static_cast<unsigned char>(line[i]))) {
            currentIndex++;
            continue;
        }
        else {
            int wordStart = i;
            while (i < (int)line.length() &&
                   !std::isspace(static_cast<unsigned char>(line[i])) &&
                   line[i] != '"') {
                i++;
            }
            std::string word = line.substr(wordStart, i - wordStart);
            TokenType type = identifyTokenType(word);
            tokens.push_back({type, wordStart, (int)word.length()});
            currentIndex += word.length();
            i--;
        }
    }
    return tokens;
}

// انتخاب رنگ برای هر نوع توکن (حالت دارک و لایت)
SDL_Color getColorForTokenType(TokenType type, bool isDarkMode) {
    if (isDarkMode) {
        switch (type) {
            case TokenType::Keyword:
                return SDL_Color{198, 120, 221, 255};
            case TokenType::DataType:
                return SDL_Color{224, 108, 117, 255};
            case TokenType::FunctionName:
                return SDL_Color{255, 255, 255, 255};
            case TokenType::VariableName:
                return SDL_Color{229, 192, 123, 255};
            case TokenType::StringLiteral:
            case TokenType::CharacterLiteral:
                return SDL_Color{152, 195, 121, 255};
            case TokenType::NumberLiteral:
                return SDL_Color{209, 154, 102, 255};
            case TokenType::Comment:
                return SDL_Color{92, 99, 112, 255};
            case TokenType::PreprocessorDirective:
                return SDL_Color{86, 182, 194, 255};
            case TokenType::Operator:
                return SDL_Color{213, 94, 0, 255};
            case TokenType::Parenthesis:
                return SDL_Color{171, 178, 191, 255};
            default:
                return SDL_Color{255, 255, 255, 255};
        }
    } else {
        // حالت روشن
        switch (type) {
            case TokenType::Keyword:
                return SDL_Color{0, 51, 102, 255};
            case TokenType::DataType:
                return SDL_Color{0, 128, 128, 255};
            case TokenType::FunctionName:
                return SDL_Color{0, 0, 0, 255};
            case TokenType::VariableName:
                return SDL_Color{0, 0, 0, 255};
            case TokenType::StringLiteral:
            case TokenType::CharacterLiteral:
                return SDL_Color{0, 100, 0, 255};
            case TokenType::NumberLiteral:
                return SDL_Color{128, 0, 128, 255};
            case TokenType::Comment:
                return SDL_Color{128, 128, 128, 255};
            case TokenType::PreprocessorDirective:
                return SDL_Color{0, 139, 139, 255};
            case TokenType::Operator:
                return SDL_Color{128, 0, 0, 255};
            case TokenType::Parenthesis:
                return SDL_Color{184, 134, 11, 255};
            default:
                return SDL_Color{0, 0, 0, 255};
        }
    }
}

// رندر کردن یک خط با رنگ‌آمیزی سینتکس
void renderLineWithSyntaxHighlighting(SDL_Renderer* renderer, TTF_Font* font, const std::string& line, int y, bool isDarkMode) {
    auto tokens = tokenizeLine(line);
    int x = 10 - horizontalScrollOffset; // فاصله از سمت چپ به همراه اسکرول افقی
    //int x = 10; // فاصله از سمت چپ

    for (const auto& token : tokens) {
        SDL_Color color = getColorForTokenType(token.type, isDarkMode);
        SDL_Surface* textSurface = TTF_RenderText_Blended(font, line.substr(token.start, token.length).c_str(), color);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        SDL_Rect renderQuad = {x, y, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);

        x += textSurface->w + 10; // فاصلهٔ بین توکن‌ها
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }
}

// تنظیم اسکرول که خط فعلی حتماً دیده شود
void ensureLastLineVisible(int currentLine, int &scrollOffset, int SCREEN_HEIGHT, int LINE_HEIGHT, int totalLines) {
    int cursorY = currentLine * LINE_HEIGHT - scrollOffset;
    if (cursorY < 0) {
        scrollOffset = currentLine * LINE_HEIGHT;
    } else if (cursorY + LINE_HEIGHT > SCREEN_HEIGHT) {
        scrollOffset = (currentLine + 1) * LINE_HEIGHT - SCREEN_HEIGHT;
    }
    int contentHeight = totalLines * LINE_HEIGHT;
    if (contentHeight > SCREEN_HEIGHT) {
        if (scrollOffset > contentHeight - SCREEN_HEIGHT) {
            scrollOffset = contentHeight - SCREEN_HEIGHT;
        }
    } else {
        scrollOffset = 0;
    }
}

//baraye inke jayi click mikonim cursor biad oonja
void SDL_MouseButtonDown(SDL_Event &e, std::vector<std::string> &lines, int &currentLine, int &cursorPos, int scrollOffset, TTF_Font *font, int LINE_HEIGHT, bool &selecting) {
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    // محاسبهٔ خطی که کلیک شده
    int clickedLine = (my + scrollOffset) / LINE_HEIGHT;
    if (clickedLine >= 0 && clickedLine < (int)lines.size()) {
        currentLine = clickedLine;

        // محاسبهٔ محل قرارگیری کرسر در خط
        int posX = mx - 10; // تنظیم فاصلهٔ از چپ
        int textWidth = 0;
        for (int i = 0; i <= (int)lines[currentLine].size(); ++i) {
            TTF_SizeText(font, lines[currentLine].substr(0, i).c_str(), &textWidth, nullptr);
            if (textWidth > posX) {
                cursorPos = i - 1;
                break;
            }
            cursorPos = i;
        }
    }

    // غیرفعال کردن حالت انتخابی
    selecting = false;
}

// تابع برای تشخیص عدم وجود ;
bool checkSemicolon(const std::string& line) {
    // بررسی اینکه خط با ; به پایان نرسد
    return !line.empty() && line.back() != ';';
}

// تابع برای تشخیص کلمات کلیدی اشتباه تایپی
bool checkSpelling(const std::string& line) {
    std::istringstream ss(line);
    std::string word;

    while (ss >> word) {
        // بررسی اینکه کلمه یک کلمه کلیدی است یا نه
        if (validKeywords.find(word) == validKeywords.end()) {
            // اگر کلمه در مجموعه کلمات کلیدی نیست، ارور می‌دهیم
            return true; // خطای املایی
        }
    }

    return false; // اگر هیچ خطای املایی نباشد
}


// تابع بررسی نام متغیرهای نامعتبر
bool checkInvalidVariableNames(const std::string& line) {
    std::istringstream ss(line);
    std::string word;

    // خواندن اولین کلمه که نوع داده است (int, float, ...)
    if (!(ss >> word)) return false;

    // بررسی اینکه آیا این کلمه در لیست کلمات کلیدی هست (یعنی یک نوع داده است)
    if (validKeywords.find(word) != validKeywords.end()) {
        std::string varName;

        // خواندن نام متغیر بعد از نوع داده
        if (ss >> varName) {

            // بررسی اینکه نام متغیر در لیست کلمات کلیدی است یا نه
            if (validKeywords.find(varName) != validKeywords.end()) {

                return true;
            }
        }
    }
    return false;
}

// تابع برای تشخیص متغیر تعریف نشده
bool checkUndefinedVariable(const std::string& line) {
    // تقسیم خط به کلمات
    std::istringstream ss(line);
    std::string word;

    // بررسی کلمات برای پیدا کردن تعریف‌های جدید
    while (ss >> word) {
        // اگر کلمه کلیدی نوع متغیر است (مثل int, float, double, ...)
        if (word == "int" || word == "float" || word == "double" || word == "char") {
            // فرض می‌کنیم که بعد از کلمه نوع، اسم متغیر قرار دارد
            ss >> word;  // دریافت اسم متغیر
            definedVariables.insert(word);  // اضافه کردن اسم متغیر به مجموعه
            return false; // ارور نمی‌دهیم چون متغیر در حال تعریف است
        } else {
            // بررسی اگر متغیر در حال استفاده تعریف نشده باشد
            if (definedVariables.find(word) == definedVariables.end()) {
                return true; // ارور برای متغیر تعریف نشده
            }
        }
    }
    return false; // اگر خطی وجود نداشته باشد که مشکل داشته باشد
}


// تابع برای بررسی رشته‌ها
bool checkStringLiteral(const std::string& line) {
    int quoteCount = std::count(line.begin(), line.end(), '"');
    return quoteCount % 2 != 0;
}

// تابع برای بررسی عملگرهای نامعتبر
bool checkInvalidOperators(const std::string& line) {
    static const std::vector<std::string> invalidOperators = {"==+", "+++"};
    for (const auto& op : invalidOperators) {
        if (line.find(op) != std::string::npos) {
            return true;
        }
    }
    return false;
}



// ساختار اکشن برای Undo/Redo
struct EditAction {
    int lineIndex;
    int cursorPos;
    std::string oldText;  // متن کامل قبل از تغییر
    std::string newText;  // متن کامل بعد از تغییر
};

// پشته‌های Undo/Redo
static std::stack<EditAction> undoStack;
static std::stack<EditAction> redoStack;


int main(int argc, char* argv[]) {

    // راه‌اندازی SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }
    // راه‌اندازی TTF
    if (TTF_Init() == -1) {
        std::cerr << "TTF could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    // ساخت پنجره
    SDL_Window* window = SDL_CreateWindow("Text Editor BP",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH,
                                          SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    //project icon
    SDL_Surface* icon = SDL_LoadBMP("C:\\Users\\sohan\\CLionProjects\\untitled61\\logo2.bmp");
    if (icon) {
        SDL_SetWindowIcon(window, icon);
        SDL_FreeSurface(icon);  // بعد از تنظیم آیکون، سطح را آزاد کن
    } else {
        std::cout << "Failed to load icon: " << SDL_GetError() << std::endl;
    }


    // ساخت Renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }
    // بارگذاری پس‌زمینه برای دارک مود و لایت مود
    darkModeBackground = IMG_LoadTexture(renderer, "C:\\Users\\sohan\\CLionProjects\\untitled61\\cmake-build-debug\\dark mode.jpg");
    lightModeBackground = IMG_LoadTexture(renderer, "C:\\Users\\sohan\\CLionProjects\\untitled61\\cmake-build-debug\\photo_2025-02-05_08-14-00.jpg");

    if (!darkModeBackground || !lightModeBackground) {
        std::cerr << "خطا در بارگذاری پس‌زمینه! SDL_Error: " << SDL_GetError() << std::endl;
    }


    // بارگذاری فونت
    TTF_Font* font = TTF_OpenFont(R"(C:\Windows\Fonts\consola.ttf)", 18);
    if (!font) {
        std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // شروع دریافت ورودی متنی (الزامی در برخی نسخه‌های SDL)
    SDL_StartTextInput();

    // متغیرهای ویرایشگر
    std::vector<std::string> lines = {""};
    int currentLine = 0;
    int cursorPos = 0;
    int scrollOffset = 0;
    const int LINE_HEIGHT = TTF_FontHeight(font);

    // برای چشمک‌زدن نشانگر
    Uint32 lastCursorToggle = SDL_GetTicks();
    bool cursorVisible = true;
    const Uint32 CURSOR_BLINK_INTERVAL = 500;

    bool isDarkMode = false;

    // دکمه‌های مختلف
    SDL_Rect toolbar= {680, 0, 120, 600};
    SDL_Rect saveButton = {690, 10, 100, 30};
    SDL_Rect newButton  = {690, 50, 100, 30};
    SDL_Rect exitButton = {690, 90, 100, 30};
    SDL_Rect modeButton = {690, 130, 100, 30};
    SDL_Rect undoBtn    = {690, 170, 100, 30};
    SDL_Rect redoBtn    = {690, 210, 100, 30};
    SDL_Rect debugButton = {690, 250, 100, 30};
    SDL_Rect runButton = {690, 290, 100, 30};


    bool showErrors = false; // برای نمایش یا عدم نمایش ارورها

    std::vector<std::string> savedFiles;

    // تابع برای بررسی کامنت‌های چندخطی
    bool isMultiLineCommentOpen = false;

    // برای انتخاب متن
    bool selecting = false;
    int selectStartLine = 0;
    int selectEndLine   = 0;
    int selectStartPos  = 0;
    int selectEndPos    = 0;
    std::string copiedText;
    int targetLine = -1;

    bool quit = false;
    std::vector<std::string> errorMessages;

    while (!quit) {
        // کنترل چشمک‌زدن نشانگر
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime > lastCursorToggle + CURSOR_BLINK_INTERVAL) {
            cursorVisible = !cursorVisible;
            lastCursorToggle = currentTime;
        }

        errorMessages.clear();
        // دریافت رویدادها
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);

                // کلیک روی دکمه Debug
                if (mx >= debugButton.x && mx <= debugButton.x + debugButton.w &&
                    my >= debugButton.y && my <= debugButton.y + debugButton.h) {
                    showErrors = !showErrors;  // وضعیت نمایش ارورها را تغییر بده
                }

                // کلیک روی دکمه Save
                if (mx >= saveButton.x && mx <= saveButton.x + saveButton.w &&
                    my >= saveButton.y && my <= saveButton.y + saveButton.h) {
                    std::string fileName;
                    std::cout << "Enter file name: ";
                    std::cin >> fileName;
                    savedFiles.push_back(fileName);
                }

                // کلیک روی دکمه Mode
                if (mx >= modeButton.x && mx <= modeButton.x + modeButton.w &&
                    my >= modeButton.y && my <= modeButton.y + modeButton.h) {
                    isDarkMode = !isDarkMode;
                }

                // کلیک روی دکمه Exit
                if (mx >= exitButton.x && mx <= exitButton.x + exitButton.w &&
                    my >= exitButton.y && my <= exitButton.y + exitButton.h) {
                    quit = true;
                }

                // کلیک روی دکمه New
                if (mx >= newButton.x && mx <= newButton.x + newButton.w &&
                    my >= newButton.y && my <= newButton.y + newButton.h) {
                    lines.clear();
                    lines.push_back("");
                    currentLine = 0;
                    cursorPos = 0;
                    scrollOffset = 0;
                    // پاک کردن پشته‌های Undo و Redo
                    while (!undoStack.empty()) undoStack.pop();
                    while (!redoStack.empty()) redoStack.pop();
                }

                // کلیک روی دکمه Undo
                if (mx >= undoBtn.x && mx <= undoBtn.x + undoBtn.w &&
                    my >= undoBtn.y && my <= undoBtn.y + undoBtn.h) {
                    if (!undoStack.empty()) {
                        EditAction action = undoStack.top();
                        undoStack.pop();

                        // برای Redo، اکشن معکوس می‌سازیم
                        EditAction redoAction;
                        redoAction.oldText   = action.oldText;
                        redoAction.newText   = action.newText;
                        redoAction.lineIndex = action.lineIndex;
                        redoAction.cursorPos = action.cursorPos;
                        redoStack.push(redoAction);

                        // برگرداندن متن به حالت قدیمی
                        lines       = split(action.oldText, '\n');
                        currentLine = action.lineIndex;
                        cursorPos   = action.cursorPos;
                    }
                }

                // کلیک روی دکمه Redo
                if (mx >= redoBtn.x && mx <= redoBtn.x + redoBtn.w &&
                    my >= redoBtn.y && my <= redoBtn.y + redoBtn.h) {
                    if (!redoStack.empty()) {
                        EditAction action = redoStack.top();
                        redoStack.pop();

                        // برای Undo، اکشن معکوس
                        EditAction undoAction;
                        undoAction.oldText   = action.oldText;
                        undoAction.newText   = action.newText;
                        undoAction.lineIndex = action.lineIndex;
                        undoAction.cursorPos = action.cursorPos;
                        undoStack.push(undoAction);

                        // اعمال متن جدید
                        lines       = split(action.newText, '\n');
                        currentLine = action.lineIndex;
                        cursorPos   = action.cursorPos;
                    }
                }

                //run button
                if (mx >= runButton.x && mx <= runButton.x + runButton.w &&
                    my >= runButton.y && my <= runButton.y + runButton.h) {
                    // ذخیره محتوای تکست ادیتور در یک فایل موقت
                    std::string filename = "temp.cpp";
                    std::ofstream file(filename);
                    if (file.is_open()) {
                        for (const auto &line : lines) {
                            file << line << "\n";
                        }
                        file.close();

                        // کامپایل
                        if (compile(filename)) {
                            std::cout << "Compilation successful!\n";
                            run("output"); // اجرای فایل کامپایل شده
                        } else {
                            std::cerr << "Compilation failed!\n";
                        }
                    } else {
                        std::cerr << "Unable to open file for writing.\n";
                    }
                }

                // شروع انتخاب متن با ماوس
                if (e.button.button == SDL_BUTTON_LEFT) {
                    SDL_MouseButtonDown(e, lines, currentLine, cursorPos, scrollOffset, font, LINE_HEIGHT, selecting);
                    //selecting = true;
                    selectStartLine = (my + scrollOffset) / LINE_HEIGHT;
                    selectEndLine   = selectStartLine;
                    selectStartPos  = 0;
                    selectEndPos    = 0;
                    if (selectStartLine >= 0 && selectStartLine < (int)lines.size()) {
                        int posX = mx - 10;
                        int textWidth = 0;
                        for (int i = 0; i <= (int)lines[selectStartLine].size(); ++i) {
                            TTF_SizeText(font, lines[selectStartLine].substr(0, i).c_str(), &textWidth, nullptr);
                            if (textWidth > posX) {
                                selectStartPos = i - 1;
                                break;
                            }
                            selectStartPos = i;
                        }
                    }
                }
            }
            else if (e.type == SDL_MOUSEBUTTONUP) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    if (!selecting) {
                        // اگر در حالت انتخاب نبودیم، انتخاب را پاک کن
                        selectStartLine = selectEndLine = currentLine;
                        selectStartPos = selectEndPos = cursorPos;
                    }
                    selecting = false;
                }
            }
            else if (e.type == SDL_MOUSEMOTION) {
                if (e.motion.state & SDL_BUTTON_LMASK) {
                    selecting = true;
                    int mouseX = e.motion.x;
                    int mouseY = e.motion.y + scrollOffset;
                    selectEndLine = mouseY / LINE_HEIGHT;

                    if (selectEndLine >= 0 && selectEndLine < (int)lines.size()) {
                        int posX = mouseX - 10;
                        int textWidth = 0;
                        for (int i = 0; i <= (int)lines[selectEndLine].size(); ++i) {
                            TTF_SizeText(font, lines[selectEndLine].substr(0, i).c_str(), &textWidth, nullptr);
                            if (textWidth > posX) {
                                selectEndPos = i - 1;
                                break;
                            }
                            selectEndPos = i;
                        }
                    }
                }
            }
            else if (e.type == SDL_MOUSEWHEEL) {
                if (e.wheel.y > 0) {
                    scrollOffset = std::max(0, scrollOffset - LINE_HEIGHT);
                } else if (e.wheel.y < 0) {
                    scrollOffset += LINE_HEIGHT;
                } else if (e.wheel.x > 0) { // اسکرول به راست
                    horizontalScrollOffset += 20;
                } else if (e.wheel.x < 0) { // اسکرول به چپ
                    horizontalScrollOffset = std::max(0, horizontalScrollOffset - 20);
                }
            }
            else if (e.type == SDL_KEYDOWN) {
                // Ctrl + Z => Undo
                if ((e.key.keysym.sym == SDLK_z) && (SDL_GetModState() & KMOD_CTRL)) {
                    if (!undoStack.empty()) {
                        EditAction action = undoStack.top();
                        undoStack.pop();

                        // معکوس برای Redo
                        EditAction redoAction;
                        redoAction.oldText   = action.oldText;
                        redoAction.newText   = action.newText;
                        redoAction.lineIndex = action.lineIndex;
                        redoAction.cursorPos = action.cursorPos;
                        redoStack.push(redoAction);

                        // برگرداندن به قدیم
                        lines       = split(action.oldText, '\n');
                        currentLine = action.lineIndex;
                        cursorPos   = action.cursorPos;
                    }
                }
                    // Ctrl + Y => Redo
                else if ((e.key.keysym.sym == SDLK_y) && (SDL_GetModState() & KMOD_CTRL)) {
                    if (!redoStack.empty()) {
                        EditAction action = redoStack.top();
                        redoStack.pop();

                        // معکوس برای Undo
                        EditAction undoAction;
                        undoAction.oldText   = action.oldText;
                        undoAction.newText   = action.newText;
                        undoAction.lineIndex = action.lineIndex;
                        undoAction.cursorPos = action.cursorPos;
                        undoStack.push(undoAction);

                        // اعمال متن جدید
                        lines       = split(action.newText, '\n');
                        currentLine = action.lineIndex;
                        cursorPos   = action.cursorPos;
                    }
                }
                    // Ctrl + C => Copy
                else if ((e.key.keysym.sym == SDLK_c && (SDL_GetModState() & KMOD_CTRL)) && selectStartLine <= selectEndLine) {
                    copiedText.clear();
                    if (selectStartLine == selectEndLine) {
                        copiedText = lines[selectStartLine].substr(selectStartPos, selectEndPos - selectStartPos);
                    } else {
                        copiedText = lines[selectStartLine].substr(selectStartPos) + "\n";
                        for (int i = selectStartLine + 1; i < selectEndLine; ++i) {
                            copiedText += lines[i] + "\n";
                        }
                        copiedText += lines[selectEndLine].substr(0, selectEndPos);
                    }
                }
                    // Ctrl + V => Paste
                else if ((e.key.keysym.sym == SDLK_v && (SDL_GetModState() & KMOD_CTRL)) && !copiedText.empty()) {
                    // اکشن برای Undo
                    EditAction action;
                    action.oldText   = join(lines, '\n');
                    action.lineIndex = currentLine;
                    action.cursorPos = cursorPos;

                    // انجام عمل Paste
                    std::string textToPaste = copiedText;
                    size_t pos = 0;
                    std::string beforeCursor = lines[currentLine].substr(0, cursorPos);
                    std::string afterCursor  = lines[currentLine].substr(cursorPos);
                    std::string line;
                    bool firstLine = true;
                    while ((pos = textToPaste.find('\n')) != std::string::npos) {
                        line = textToPaste.substr(0, pos);
                        if (firstLine) {
                            lines[currentLine] = beforeCursor + line;
                            firstLine = false;
                        } else {
                            lines.insert(lines.begin() + currentLine + 1, line);
                            currentLine++;
                        }
                        textToPaste.erase(0, pos + 1);
                    }
                    lines.insert(lines.begin() + currentLine + 1, textToPaste + afterCursor);
                    currentLine++;
                    cursorPos = (int)textToPaste.size();

                    // ثبت در UndoStack
                    action.newText = join(lines, '\n');
                    undoStack.push(action);
                    // تغییر جدید => پاک شدن Redo
                    while (!redoStack.empty()) redoStack.pop();
                }
                    // Ctrl + X => Cut
                else if ((e.key.keysym.sym == SDLK_x && (SDL_GetModState() & KMOD_CTRL)) && selectStartLine <= selectEndLine) {
                    // اکشن برای Undo
                    EditAction action;
                    action.oldText   = join(lines, '\n');
                    action.lineIndex = currentLine;
                    action.cursorPos = cursorPos;

                    // بریدن متن
                    copiedText.clear();
                    if (selectStartLine == selectEndLine) {
                        copiedText = lines[selectStartLine].substr(selectStartPos, selectEndPos - selectStartPos);
                        lines[selectStartLine].erase(selectStartPos, selectEndPos - selectStartPos);
                    } else {
                        copiedText = lines[selectStartLine].substr(selectStartPos);
                        lines[selectStartLine].erase(selectStartPos);
                        for (int i = selectStartLine + 1; i < selectEndLine; ++i) {
                            copiedText += "\n" + lines[i];
                        }
                        copiedText += "\n" + lines[selectEndLine].substr(0, selectEndPos);
                        lines[selectEndLine].erase(0, selectEndPos);
                        lines.erase(lines.begin() + selectStartLine + 1, lines.begin() + selectEndLine);
                    }

                    // ثبت در UndoStack
                    action.newText = join(lines, '\n');
                    undoStack.push(action);
                    // پاک شدن Redo
                    while (!redoStack.empty()) redoStack.pop();
                }
                    // Ctrl + S => ذخیره
                else if ((e.key.keysym.sym == SDLK_s && (SDL_GetModState() & KMOD_CTRL))) {
                    std::string fileName;
                    std::cout << "Enter file name: ";
                    std::cin >> fileName;
                    savedFiles.push_back(fileName);
                }
                    // Ctrl + A => انتخاب همه
                else if ((e.key.keysym.sym == SDLK_a && (SDL_GetModState() & KMOD_CTRL))) {
                    selectStartLine = 0;
                    selectStartPos  = 0;
                    selectEndLine   = (int)lines.size() - 1;
                    selectEndPos    = (int)lines[selectEndLine].size();
                    if (selectEndLine == 0) {
                        selectEndPos = (int)lines[0].size();
                    }
                }
                    // Ctrl + G => پرش به خط
                else if ((e.key.keysym.sym == SDLK_g && (SDL_GetModState() & KMOD_CTRL))) {
                    std::cout << "Enter line number: ";
                    std::cin >> targetLine;
                    if (targetLine >= 1 && targetLine <= (int)lines.size()) {
                        currentLine = targetLine - 1;
                        cursorPos = 0;
                        ensureLastLineVisible(currentLine, scrollOffset, SCREEN_HEIGHT, LINE_HEIGHT, (int)lines.size());
                    } else {
                        std::cout << "Invalid line number.\n";
                        targetLine = -1;
                    }
                }

                    //run ba ctrl+r
                else if ((e.key.keysym.sym == SDLK_r && (SDL_GetModState() & KMOD_CTRL))) {
                    // ذخیره محتوای تکست ادیتور در یک فایل موقت
                    std::string filename = "temp.cpp";
                    std::ofstream file(filename);
                    if (file.is_open()) {
                        for (const auto &line : lines) {
                            file << line << "\n";
                        }
                        file.close();

                        // کامپایل
                        if (compile(filename)) {
                            std::cout << "Compilation successful!\n";
                            run("output"); // اجرای فایل کامپایل شده
                        } else {
                            std::cerr << "Compilation failed!\n";
                        }
                    } else {
                        std::cerr << "Unable to open file for writing.\n";
                    }
                }

                    // Backspace
                else if (e.key.keysym.sym == SDLK_BACKSPACE) {
                    // اکشن برای Undo
                    EditAction action;
                    action.oldText   = join(lines, '\n');
                    action.lineIndex = currentLine;
                    action.cursorPos = cursorPos;

                    // حذف کاراکتر
                    if (cursorPos > 0) {
                        lines[currentLine].erase(cursorPos - 1, 1);
                        cursorPos--;
                    } else if (currentLine > 0) {
                        cursorPos = (int)lines[currentLine - 1].size();
                        lines[currentLine - 1] += lines[currentLine];
                        lines.erase(lines.begin() + currentLine);
                        currentLine--;
                    }
                    if (lines.empty()) {
                        lines.push_back("");
                        currentLine = 0;
                        cursorPos = 0;
                    }

                    // ثبت تغییر
                    action.newText = join(lines, '\n');
                    undoStack.push(action);
                    while (!redoStack.empty()) redoStack.pop();
                }
                    // Enter
                else if (e.key.keysym.sym == SDLK_RETURN) {
                    // اکشن برای Undo
                    EditAction action;
                    action.oldText   = join(lines, '\n');
                    action.lineIndex = currentLine;
                    action.cursorPos = cursorPos;

                    // شکستن خط
                    std::string remainder = lines[currentLine].substr(cursorPos);
                    lines[currentLine] = lines[currentLine].substr(0, cursorPos);
                    lines.insert(lines.begin() + currentLine + 1, remainder);
                    currentLine++;
                    cursorPos = 0;
                    ensureLastLineVisible(currentLine, scrollOffset, SCREEN_HEIGHT, LINE_HEIGHT, (int)lines.size());

                    // ثبت تغییر
                    action.newText = join(lines, '\n');
                    undoStack.push(action);
                    while (!redoStack.empty()) redoStack.pop();
                }
                    // Tab
                else if (e.key.keysym.sym == SDLK_TAB) {
                    // اکشن برای Undo
                    EditAction action;
                    action.oldText   = join(lines, '\n');
                    action.lineIndex = currentLine;
                    action.cursorPos = cursorPos;

                    // افزودن 4 فاصله
                    lines[currentLine].insert(cursorPos, "    ");
                    cursorPos += 4;

                    // ثبت تغییر
                    action.newText = join(lines, '\n');
                    undoStack.push(action);
                    while (!redoStack.empty()) redoStack.pop();
                }
                    // حرکت مکان‌نما
                else if (e.key.keysym.sym == SDLK_LEFT) {
                    if (cursorPos > 0) {
                        cursorPos--;
                    } else if (currentLine > 0) {
                        currentLine--;
                        cursorPos = (int)lines[currentLine].size();
                    }
                }
                else if (e.key.keysym.sym == SDLK_RIGHT) {
                    if (cursorPos < (int)lines[currentLine].size()) {
                        cursorPos++;
                    } else if (currentLine < (int)lines.size() - 1) {
                        currentLine++;
                        cursorPos = 0;
                    }
                }
                else if (e.key.keysym.sym == SDLK_UP) {
                    if (currentLine > 0) {
                        currentLine--;
                        cursorPos = std::min(cursorPos, (int)lines[currentLine].size());
                        ensureLastLineVisible(currentLine, scrollOffset, SCREEN_HEIGHT, LINE_HEIGHT, (int)lines.size());
                    }
                }
                else if (e.key.keysym.sym == SDLK_DOWN) {
                    if (currentLine < (int)lines.size() - 1) {
                        currentLine++;
                        cursorPos = std::min(cursorPos, (int)lines[currentLine].size());
                        ensureLastLineVisible(currentLine, scrollOffset, SCREEN_HEIGHT, LINE_HEIGHT, (int)lines.size());
                    }
                }
            }
                // وقتی کاربر متن جدید تایپ می‌کند (رویداد متنی)
            else if (e.type == SDL_TEXTINPUT) {
                // اکشن برای Undo
                EditAction action;
                action.oldText   = join(lines, '\n');
                action.lineIndex = currentLine;
                action.cursorPos = cursorPos;

                // درج کاراکترهای تایپ شده
                char ch = e.text.text[0];
                if (ch == '(' || ch == '{' || ch == '[') {   //takmil khodkar parantez
                    std::string toInsert(1, ch);
                    toInsert += " ";
                    if (ch == '(') {
                        toInsert += ")";
                    } else if (ch == '{') {
                        toInsert += "}";
                    } else if (ch == '[') {
                        toInsert += "]";
                    }
                    lines[currentLine].insert(cursorPos, toInsert);
                    cursorPos += 2; // مکان کرسر را به وسط پرانتز یا کروشه منتقل کن
                } else {
                    lines[currentLine].insert(cursorPos, e.text.text);
                    cursorPos += (int)strlen(e.text.text);
                }
                ensureLastLineVisible(currentLine, scrollOffset, SCREEN_HEIGHT, LINE_HEIGHT, (int)lines.size());

                // ثبت تغییر
                action.newText = join(lines, '\n');
                undoStack.push(action);
                while (!redoStack.empty()) redoStack.pop();
            }
        }

        // تشخیص خطاها برای هر خط
        for (int i = 0; i < (int)lines.size(); ++i) {
            if (checkSemicolon(lines[i])) {
                errorMessages.push_back("Error: Missing semicolon at line " + std::to_string(i + 1));
            }
            /*if (checkSpelling(lines[i])) {
                errorMessages.push_back("Error: Misspelled keyword at line " + std::to_string(i + 1));
            }*/
            if (checkInvalidVariableNames(lines[i])) {
                errorMessages.push_back("Error: Incorrect variable naming at line " + std::to_string(i + 1));
            }
            /*if (checkUndefinedVariable(lines[i])) {
                errorMessages.push_back("Error: Undefined variable at line " + std::to_string(i + 1));
            }*/
            if (checkStringLiteral(lines[i])) {
                errorMessages.push_back("Error: Unclosed string literal at line " + std::to_string(i + 1));
            }
            if (checkInvalidOperators(lines[i])) {
                errorMessages.push_back("Error: Invalid operator at line " + std::to_string(i + 1));
            }
            if (lines[i].find("/*") != std::string::npos) {
                isMultiLineCommentOpen = true; // شروع کامنت چندخطی
            }
            if (lines[i].find("*/") != std::string::npos) {
                if (!isMultiLineCommentOpen) {
                    errorMessages.push_back("Error: Unmatched '*/' at line " + std::to_string(i + 1));
                } else {
                    isMultiLineCommentOpen = false; // بستن کامنت چندخطی
                }
            }

        }

        std::vector<std::string> warnings = detectRepeatedIfElse(lines);
        errorMessages.insert(errorMessages.end(), warnings.begin(), warnings.end());

        //operator error
        for (int i = 0; i < (int)lines.size(); ++i) {
            std::string line = lines[i];

            if (line.empty()) continue; // اگر خط خالی است، رد کن

            for (size_t j = 0; j < line.length(); ++j) {
                if (ispunct(line[j])) { // اگر کاراکتر یک علامت (عملگر) بود
                    std::string op(1, line[j]);

                    // تا زمانی که کاراکترهای بعدی هم عملگر باشند، به `op` اضافه کن
                    size_t k = j + 1;
                    while (k < line.length() && ispunct(line[k])) {
                        op += line[k];
                        ++k;
                    }

                    // اگر عملگر نامعتبر بود، ارور بده
                    if (!isValidOperator(op)) {
                        errorMessages.push_back("Error: Invalid operator '" + op + "' at line " + std::to_string(i + 1));
                    }

                    // جلو بردن `j` تا از بررسی مجدد همان کاراکترها جلوگیری شود
                    j = k - 1;
                }
            }
        }




        // رسم پس‌زمینه
        SDL_SetRenderDrawColor(renderer,
                               isDarkMode ? 0   : 255,
                               isDarkMode ? 0   : 255,
                               isDarkMode ? 0   : 255,
                               255);
        SDL_RenderClear(renderer);
        SDL_Texture* currentBackground = isDarkMode ? darkModeBackground : lightModeBackground;
        if (currentBackground) {
            SDL_RenderCopy(renderer, currentBackground, NULL, NULL);
        } // برای نشان دادن بک گراند
        // رسم خطوط
        int drawY = -scrollOffset;
        for (int i = 0; i < (int)lines.size(); ++i) {
            if (drawY + LINE_HEIGHT > 0 && drawY < SCREEN_HEIGHT) {
                // رندر کردن هر خط با رنگ‌آمیزی
                renderLineWithSyntaxHighlighting(renderer, font, lines[i], drawY, isDarkMode);

                // مشخص‌کردن ناحیه انتخاب‌شده
                SDL_Color selectionColor = {173, 216, 230, 150};
                bool lineIsInSelection = false;
                // تشخیص آیا این خط در محدوده انتخاب هست
                if ((selectStartLine <= i && i <= selectEndLine) ||
                    (selectEndLine <= i && i <= selectStartLine)) {
                    lineIsInSelection = true;
                }

                if (lineIsInSelection) {
                    int selStartX = 0;
                    int selEndX   = 0;

                    if (i == selectStartLine) {
                        TTF_SizeText(font, lines[i].substr(0, selectStartPos).c_str(), &selStartX, nullptr);
                    }
                    if (i == selectEndLine) {
                        TTF_SizeText(font, lines[i].substr(0, selectEndPos).c_str(), &selEndX, nullptr);
                    }
                    else if (i > std::min(selectStartLine, selectEndLine) &&
                             i < std::max(selectStartLine, selectEndLine)) {
                        // کل خط انتخاب شده
                        TTF_SizeText(font, lines[i].c_str(), &selEndX, nullptr);
                    }
                    if (selectStartLine == selectEndLine) {
                        // کل انتخاب در یک خط
                        selStartX = 0;
                        TTF_SizeText(font, lines[i].substr(0, selectStartPos).c_str(), &selStartX, nullptr);
                    }

                    SDL_Rect selectRect = {
                            10 + selStartX,
                            drawY,
                            selEndX - selStartX,
                            LINE_HEIGHT
                    };
                    // رسم مستطیل شفاف انتخاب
                    SDL_SetRenderDrawColor(renderer, selectionColor.r, selectionColor.g, selectionColor.b, selectionColor.a);
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                    SDL_RenderFillRect(renderer, &selectRect);
                }

                // رسم نشانگر (کرسر)
                if (i == currentLine && cursorVisible) {
                    int cursorX = 0;
                    if (cursorPos > 0) {
                        TTF_SizeText(font, lines[currentLine].substr(0, cursorPos).c_str(), &cursorX, nullptr);
                    }
                    cursorX += 10;  // فاصله از چپ

                    // رنگ کرسر
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    // توجه: چون ما drawY را از scrollOffset کم کرده‌ایم،
                    // دیگر لازم نیست دوباره -scrollOffset کنیم
                    SDL_RenderDrawLine(renderer,
                                       cursorX, drawY,
                                       cursorX, drawY + LINE_HEIGHT);
                }
            }
            drawY += LINE_HEIGHT;
        }

        //toolbar
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderFillRect(renderer, &toolbar);

        // رسم پیغام‌های خطا
        if (showErrors) {
            SDL_Rect errorBox = {0, 400, 680, 200};
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            SDL_RenderFillRect(renderer, &errorBox);

            int errorY = 410;
            for (const auto& message : errorMessages) {
                SDL_Surface* errorSurf = TTF_RenderText_Blended(font, message.c_str(), {255, 255, 255, 255});
                SDL_Texture* errorTex = SDL_CreateTextureFromSurface(renderer, errorSurf);
                SDL_Rect errorRect = {15, errorY, errorSurf->w, errorSurf->h};
                SDL_RenderCopy(renderer, errorTex, NULL, &errorRect);
                SDL_FreeSurface(errorSurf);
                SDL_DestroyTexture(errorTex);
                errorY += errorSurf->h + 5;
            }
        }


        // رسم دکمه‌ها و نوشته‌هایشان
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Save: آبی
        SDL_RenderFillRect(renderer, &saveButton);
        {
            SDL_Color btnColor = {255, 255, 255, 255};
            SDL_Surface* surf = TTF_RenderText_Blended(font, "Save", btnColor);
            SDL_Texture* tex  = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect dst = {
                    saveButton.x + (saveButton.w - surf->w) / 2,
                    saveButton.y + (saveButton.h - surf->h) / 2,
                    surf->w, surf->h
            };
            SDL_RenderCopy(renderer, tex, NULL, &dst);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255); // New: بنفش
        SDL_RenderFillRect(renderer, &newButton);
        {
            SDL_Color btnColor = {255, 255, 255, 255};
            SDL_Surface* surf = TTF_RenderText_Blended(font, "New", btnColor);
            SDL_Texture* tex  = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect dst = {
                    newButton.x + (newButton.w - surf->w) / 2,
                    newButton.y + (newButton.h - surf->h) / 2,
                    surf->w, surf->h
            };
            SDL_RenderCopy(renderer, tex, NULL, &dst);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Exit: قرمز
        SDL_RenderFillRect(renderer, &exitButton);
        {
            SDL_Color btnColor = {255, 255, 255, 255};
            SDL_Surface* surf = TTF_RenderText_Blended(font, "Exit", btnColor);
            SDL_Texture* tex  = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect dst = {
                    exitButton.x + (exitButton.w - surf->w) / 2,
                    exitButton.y + (exitButton.h - surf->h) / 2,
                    surf->w, surf->h
            };
            SDL_RenderCopy(renderer, tex, NULL, &dst);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Mode: خاکستری
        SDL_RenderFillRect(renderer, &modeButton);
        {
            SDL_Color btnColor = {255, 255, 255, 255};
            SDL_Surface* surf = TTF_RenderText_Blended(font, "Mode", btnColor);
            SDL_Texture* tex  = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect dst = {
                    modeButton.x + (modeButton.w - surf->w) / 2,
                    modeButton.y + (modeButton.h - surf->h) / 2,
                    surf->w, surf->h
            };
            SDL_RenderCopy(renderer, tex, NULL, &dst);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
        }

        SDL_SetRenderDrawColor(renderer, 200, 200, 0, 255); // Undo: زرد
        SDL_RenderFillRect(renderer, &undoBtn);
        {
            SDL_Color btnColor = {255, 255, 255, 255};
            SDL_Surface* surf = TTF_RenderText_Blended(font, "Undo", btnColor);
            SDL_Texture* tex  = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect dst = {
                    undoBtn.x + (undoBtn.w - surf->w) / 2,
                    undoBtn.y + (undoBtn.h - surf->h) / 2,
                    surf->w, surf->h
            };
            SDL_RenderCopy(renderer, tex, NULL, &dst);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
        }

        SDL_SetRenderDrawColor(renderer, 52, 100, 150, 255); // Redo: آبی تیره
        SDL_RenderFillRect(renderer, &redoBtn);
        {
            SDL_Color btnColor = {255, 255, 255, 255};
            SDL_Surface* surf = TTF_RenderText_Blended(font, "Redo", btnColor);
            SDL_Texture* tex  = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect dst = {
                    redoBtn.x + (redoBtn.w - surf->w) / 2,
                    redoBtn.y + (redoBtn.h - surf->h) / 2,
                    surf->w, surf->h
            };
            SDL_RenderCopy(renderer, tex, NULL, &dst);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
        }


        SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255); // debug
        SDL_RenderFillRect(renderer, &debugButton);
        {
            SDL_Color btnColor = {255, 255, 255, 255};
            SDL_Surface* surf = TTF_RenderText_Blended(font, "Debug", btnColor);
            SDL_Texture* tex  = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect dst = {
                    debugButton.x + (debugButton.w - surf->w) / 2,
                    debugButton.y + (debugButton.h - surf->h) / 2,
                    surf->w, surf->h
            };
            SDL_RenderCopy(renderer, tex, NULL, &dst);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
        }

        //run button
        SDL_SetRenderDrawColor(renderer, 219, 112, 147, 255); // debug
        SDL_RenderFillRect(renderer, &runButton);
        {
            SDL_Color btnColor = {255, 255, 255, 255};
            SDL_Surface* surf = TTF_RenderText_Blended(font, "Run", btnColor);
            SDL_Texture* tex  = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect dst = {
                    runButton.x + (runButton.w - surf->w) / 2,
                    runButton.y + (runButton.h - surf->h) / 2,
                    surf->w, surf->h
            };
            SDL_RenderCopy(renderer, tex, NULL, &dst);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
        }

        // لیست فایل‌های ذخیره‌شده (در پایین صفحه)
        int listY = 570;
        SDL_Color listColor = {0, 0, 0, 255};

        if (savedFiles.empty()) {
            // نمایش پیغام "no saved files" اگر لیست خالی است
            SDL_Surface* surf = TTF_RenderText_Blended(font, "no project", listColor);
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect dst = {685, listY, surf->w, surf->h};
            SDL_RenderCopy(renderer, tex, NULL, &dst);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
        } else {
            for (auto &file : savedFiles) {
                std::string fileNameWithExtension = file + ".cpp"; // اضافه کردن پسوند .cpp به نام فایل
                SDL_Surface* surf = TTF_RenderText_Blended(font, fileNameWithExtension.c_str(), listColor);
                SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_Rect dst = {685, listY, surf->w, surf->h};
                SDL_RenderCopy(renderer, tex, NULL, &dst);
                listY -= (surf->h + 20);
                SDL_FreeSurface(surf);
                SDL_DestroyTexture(tex);
            }
        }


        // به‌روزرسانی رندر
        SDL_RenderPresent(renderer);
    }

    // آزادسازی‌ها
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    if (darkModeBackground) SDL_DestroyTexture(darkModeBackground);
    if (lightModeBackground) SDL_DestroyTexture(lightModeBackground);

    SDL_Quit();

    return 0;
}