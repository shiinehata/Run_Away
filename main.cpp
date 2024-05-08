#include "SDLControl.h"

// Init
SDL_Window* window = initSDL(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
SDL_Renderer* renderer = createRenderer(window);

// Sounds
Mix_Chunk* buttonSound = loadSound("Resources/Sounds/Button.mp3");
Mix_Chunk* swordCollideSound = loadSound("Resources/Sounds/swordCollide.mp3");
Mix_Chunk* swordSliceSound = loadSound("Resources/Sounds/swordSlice.mp3");
Mix_Chunk* Siu = loadSound("Resources/Sounds/Siu.mp3");

// Background
SDL_Texture* Background = loadTexture("Resources/Images/Background.png", renderer);

// Font
TTF_Font* Calibri = loadFont("Resources/Fonts/calibri.ttf", 120);

// Rule
SDL_Texture* Rule = loadTexture("Resources/Images/rule.png", renderer);

struct Point
{
    int x, y;

    bool operator < (const Point& other) const
    {
        if (x == other.x) { return y < other.y; }
        else { return x < other.x; }
    }

    bool operator == (const Point& other) const
        {
            return (x == other.x) && (y == other.y);
        }
};

struct Character
{
    Point center;
    int swordCount;
    set<Point> body;
    vector<double> swordAngle;
    vector<set<Point>> sword;
    SDL_Color color;

    void swordInit()
    {
        sword.clear();
        for (int i = 0; i < swordCount; i++)
        {   
            set<Point> pointSet;
            if (swordAngle[i] < -1e3) 
                {
                    sword.push_back(pointSet);
                    continue; 
                }

            int x1 = center.x + 70 * cos(swordAngle[i]);
            int y1 = center.y + 70 * sin(swordAngle[i]);

            int x2 = x1 + DEFAULT_SWORD_LENGTH * cos(swordAngle[i]);
            int y2 = y1 + DEFAULT_SWORD_LENGTH * sin(swordAngle[i]);

            int dx = x2 - x1;
            int dy = y2 - y1;

            int steps = max(abs(dx), abs(dy));

            double xIncrement = dx / (double)steps;
            double yIncrement = dy / (double)steps;

            double x = x1;
            double y = y1;

            for (int i = 0; i <= steps; i++)
            {
                pointSet.insert({ (int)round(x), (int)round(y) });
                x += xIncrement;
                y += yIncrement;
            }

            sword.push_back(pointSet);
        }
    }

    Character(int _x, int _y, int _swordCount, SDL_Color _color)
    {
        center = { _x, _y };
        swordCount = _swordCount;
        color = _color;

        // Body init
        vector<Point> pointList;
        for (int radius = 50; radius <= 60; radius++)
        {
            int x = radius - 1;
            int y = 0;
            int dx = 1;
            int dy = 1;
            int err = dx - radius * 2;
            while (x >= y)
            {
                pointList.push_back({ center.x + x, center.y + y });
                pointList.push_back({ center.x + y, center.y + x });
                pointList.push_back({ center.x - y, center.y + x });
                pointList.push_back({ center.x - x, center.y + y });
                pointList.push_back({ center.x - x, center.y - y });
                pointList.push_back({ center.x - y, center.y - x });
                pointList.push_back({ center.x + y, center.y - x });
                pointList.push_back({ center.x + x, center.y - y });

                if (err <= 0)
                {
                    y++;
                    err += dy;
                    dy += 2;
                }
                else
                {
                    x--;
                    dx += 2;
                    err += dx - radius * 2;
                }
            }
        }

        body.clear();
        for (int i = 0; i < pointList.size(); i++)
            for (int j = i + 1; j < pointList.size(); j++)
            {
                Point u = pointList[i], v = pointList[j];

                if ((u.x == v.x) && (abs(u.y - v.y) <= 10))
                {
                    for (int i = min(u.y, v.y); i <= max(u.y, v.y); i++) { body.insert({ u.x, i }); }
                }
            }

        // Swords init
        if (swordCount > 0)
            {
                swordAngle.push_back(randLD(0.0, 2 * PI, 6));
                for (int i = 1; i < swordCount; i++) { swordAngle.push_back(swordAngle.back() + (2.0 * PI / (double)(swordCount))); }
            
                swordInit();
            }
    }

    void rotate()
        {
            for (int i = 0; i < swordCount; i++) { swordAngle[i] -= DEFAULT_ANGLE_STEP; }

            swordInit();
        }   

    void draw()
    {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
        for (Point i : body) { SDL_RenderDrawPoint(renderer, i.x, i.y); }
        for (set<Point> s : sword) 
            if (!s.empty())
                { 
                    for(Point i : s) { SDL_RenderDrawPoint(renderer, i.x, i.y); }
                }
    }

    void moveLeft()
    {
        if (center.x - DEFAULT_STEP < 60) { return; }

        center.x -= DEFAULT_STEP;

        set<Point> newBody;
        for (Point i : body) { newBody.insert({ i.x - DEFAULT_STEP, i.y }); }

        body = newBody;
    }

    void moveRight()
    {
        if (center.x + DEFAULT_STEP > SCREEN_WIDTH - 60) { return; }

        center.x += DEFAULT_STEP;

        set<Point> newBody;
        for (Point i : body) { newBody.insert({ i.x + DEFAULT_STEP, i.y }); }

        body = newBody;
    }

    void moveUp()
    {
        if (center.y - DEFAULT_STEP < 60) { return; }

        center.y -= DEFAULT_STEP;

        set<Point> newBody;
        for (Point i : body) { newBody.insert({ i.x, i.y - DEFAULT_STEP }); }

        body = newBody;
    }

    void moveDown()
    {
        if (center.y + DEFAULT_STEP > SCREEN_HEIGHT - 60) { return; }

        center.y += DEFAULT_STEP;

        set<Point> newBody;
        for (Point i : body) { newBody.insert({ i.x, i.y + DEFAULT_STEP }); }

        body = newBody;
    }
};

vector<Character> c;
int Score[11], currentScore;

void loadScore()
{
    ifstream inp("Resources/High Scores.txt");

    for(int i = 0; i < 10; i++) { inp >> Score[i]; }

    inp.close();
}

bool loadLastGame()
{
    ifstream inp("Resources/Last Game.txt");
    
    int flag;
    inp >> flag;

    if (flag == 0) { SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Notification", "No last game available.", NULL); return 0; }
    
    c.clear();

    int n;
    inp >> n;

    int Cx, Cy;
    int sC;
    vector<double> sA;
    SDL_Color cl;

    for (int i = 0; i < n; i++)
    {
        inp >> Cx >> Cy;
        inp >> sC;
        sA.clear();
        for (int j = 0; j < sC; j++) 
            { 
                double x;
                inp >> x;
                sA.push_back(x);
            }
        int _r, _g, _b;
        inp >> _r >> _g >> _b;
        cl = {(unsigned char)_r, (unsigned char)_g, (unsigned char)_b};

        Character k(Cx, Cy, sC, cl);
        k.swordAngle = sA;
        k.swordInit();
    
        c.push_back(k);
    }
    inp >> currentScore;

    inp.close();
    
    return true;
}

bool pointInRect(Point u, SDL_Rect v)
{
    return ((v.x <= u.x) && (u.x <= v.x + v.w) && (v.y <= u.y) && (u.y <= v.y + v.h));
}

void menu();
void game();
void mainCharacterInit();
void saveLastGame();

void printText(SDL_Renderer* renderer, const char* message, SDL_Color color, int Size, int x, int y, int w, int h)
{
    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Calibri, message, color);
    SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    SDL_Rect Message_rect;
    Message_rect.x = x;
    Message_rect.y = y;
    Message_rect.w = w;
    Message_rect.h = h;

    SDL_RenderCopy(renderer, Message, NULL, &Message_rect);

    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(Message);
}

void help()
{
    // Background
    SDL_Texture* Background = loadTexture("Resources/Images/Background.png", renderer);
    SDL_RenderCopy(renderer, Background, NULL, NULL);

    // Caption
    printText(renderer, "HELP", White, 120, SCREEN_WIDTH / 2 - 250, 50, 500, 120);

    // Score
    currentScore = 0;

    // button
    SDL_Rect button[1][4];

    button[0][0] = { 50, 750, 400, 100 };
    button[0][1] = { 55, 755, 390, 90 };
    button[0][2] = { 60, 760, 380, 80 };
    button[0][3] = { 65, 765, 370, 70 };

    for (int i = 0; i < 1; i++)
        for (int j = 0; j < 4; j++)
        {
            if (j % 2 == 0) { SDL_SetRenderDrawColor(renderer, White.r, White.g, White.b, 255); }
            else { SDL_SetRenderDrawColor(renderer, Black.r, Black.g, Black.b, 255); }
            SDL_RenderFillRect(renderer, &button[i][j]);
        }

    const char* buttonText[1] =
    {
        " << MENU",
    };
    for (int i = 0; i < 1; i++) { printText(renderer, buttonText[i], White, 100, button[i][3].x + 10, button[i][3].y + 10, button[i][3].w - 20, button[i][3].h - 20); }

    // text background
    SDL_Rect textBackground[4];
    textBackground[0] = { 200, 200, SCREEN_WIDTH - 400, 500 };
    textBackground[1] = { 205, 205, SCREEN_WIDTH - 410, 490 };
    textBackground[2] = { 210, 210, SCREEN_WIDTH - 420, 480 };
    textBackground[3] = { 215, 215, SCREEN_WIDTH - 430, 470 };
    for (int j = 0; j < 4; j++)
    {
        if (j % 2 == 0) { SDL_SetRenderDrawColor(renderer, White.r, White.g, White.b, 255); }
        else { SDL_SetRenderDrawColor(renderer, Black.r, Black.g, Black.b, 255); }
        SDL_RenderFillRect(renderer, &textBackground[j]);
    }
    
    SDL_Rect dsRect = textBackground[3];
    SDL_RenderCopy(renderer, Rule, NULL, &dsRect);

    SDL_RenderPresent(renderer);

    SDL_Event event;
    while (true)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    int b = -1;
                    for (int i = 0; i < 5; i++)
                    {
                        if (pointInRect({ event.button.x, event.button.y }, button[i][0])) { b = i; break; }
                    }
                    if (b >= 0) { playSound(buttonSound); }
                    if (b == 0) { menu(); return; }
                }
            }
            else if (event.type == SDL_QUIT)
            {
                playSound(buttonSound);
                SDL_Delay(1000);
                quitSDL(window, renderer);
                return;
            }
        }
    }
}

void highScores()
{
    // Background
    SDL_Texture* Background = loadTexture("Resources/Images/Background.png", renderer);
    SDL_RenderCopy(renderer, Background, NULL, NULL);

    // Caption
    SDL_Rect button[1][4];

    button[0][0] = { 50, 750, 400, 100 };
    button[0][1] = { 55, 755, 390, 90 };
    button[0][2] = { 60, 760, 380, 80 };
    button[0][3] = { 65, 765, 370, 70 };

    for (int i = 0; i < 1; i++)
        for (int j = 0; j < 4; j++)
        {
            if (j % 2 == 0) { SDL_SetRenderDrawColor(renderer, White.r, White.g, White.b, 255); }
            else { SDL_SetRenderDrawColor(renderer, Black.r, Black.g, Black.b, 255); }
            SDL_RenderFillRect(renderer, &button[i][j]);
        }

    const char* buttonText[1] =
    {
        " << MENU",
    };
    for (int i = 0; i < 1; i++) { printText(renderer, buttonText[i], White, 100, button[i][3].x + 10, button[i][3].y + 10, button[i][3].w - 20, button[i][3].h - 20); }

    // Present scores
    SDL_Rect scoreBox[10][4];
    scoreBox[0][0] = { SCREEN_WIDTH / 2 - 500, 50, 400, 100 };
    scoreBox[0][1] = { SCREEN_WIDTH / 2 - 495, 55, 390, 90 };
    scoreBox[0][2] = { SCREEN_WIDTH / 2 - 490, 60, 380, 80 };
    scoreBox[0][3] = { SCREEN_WIDTH / 2 - 485, 65, 370, 70 };
    scoreBox[1][0] = { SCREEN_WIDTH / 2 + 100, 50, 400, 100 };
    scoreBox[1][1] = { SCREEN_WIDTH / 2 + 105, 55, 390, 90 };
    scoreBox[1][2] = { SCREEN_WIDTH / 2 + 110, 60, 380, 80 };
    scoreBox[1][3] = { SCREEN_WIDTH / 2 + 115, 65, 370, 70 };
    for (int i = 2; i < 10; i++)
        {
            scoreBox[i][0] = { scoreBox[i - 2][0].x, scoreBox[i - 2][0].y + 120, 400, 100};
            scoreBox[i][1] = { scoreBox[i - 2][1].x, scoreBox[i - 2][1].y + 120, 390, 90 };
            scoreBox[i][2] = { scoreBox[i - 2][2].x, scoreBox[i - 2][2].y + 120, 380, 80 };
            scoreBox[i][3] = { scoreBox[i - 2][3].x, scoreBox[i - 2][3].y + 120, 370, 70 };
        }

    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 4; j++)
        {
            if (j % 2 == 0) { SDL_SetRenderDrawColor(renderer, White.r, White.g, White.b, 255); }
            else { SDL_SetRenderDrawColor(renderer, Black.r, Black.g, Black.b, 255); }
            SDL_RenderFillRect(renderer, &scoreBox[i][j]);
        }

    string scoreText[10];
    for (int i = 0; i < 10; i++)
    {
        scoreText[i] = "Top " + to_string(i + 1) + ": " + to_string(Score[i]);
    }
    
    for (int i = 0; i < 10; i++) { printText(renderer, scoreText[i].c_str(), White, 100, scoreBox[i][3].x + 10, scoreBox[i][3].y + 10, scoreBox[i][3].w - 20, scoreBox[i][3].h - 20); }

    SDL_RenderPresent(renderer);

    SDL_Event event;
    while (true)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    int b = -1;
                    for (int i = 0; i < 5; i++)
                    {
                        if (pointInRect({ event.button.x, event.button.y }, button[i][0])) { b = i; break; }
                    }
                    if (b >= 0) { playSound(buttonSound); }
                    if (b == 0) { menu(); return; }
                }
            }
            else if (event.type == SDL_QUIT)
            {
                playSound(buttonSound);
                SDL_Delay(1000);
                quitSDL(window, renderer);
                return;
            }
        }
    }
}

void total()
{
    // Update
    c.clear();
    Score[10] = currentScore;
    sort(Score, Score + 11, greater<int> ());
    saveLastGame();

    // Background
    SDL_Texture* Background = loadTexture("Resources/Images/Background.png", renderer);
    SDL_RenderCopy(renderer, Background, NULL, NULL);

    // Caption
    printText(renderer, "TOTAL", White, 120, SCREEN_WIDTH / 2 - 250, 50, 500, 120);

    // Score
    string Txt1 = "HIGHEST SCORE: " + to_string(max(currentScore, Score[0]));
    string Txt2 = "YOUR SCORE: " + to_string(currentScore);
    printText(renderer, Txt1.c_str(), White, 70, SCREEN_WIDTH / 2 - 150, 200, 300, 70);
    printText(renderer, Txt2.c_str(), White, 70, SCREEN_WIDTH / 2 - 150, 280, 300, 70);

    if (currentScore == Score[0])
        {
            printText(renderer, "YOU ARE NOW TOP 1", White, 70, SCREEN_WIDTH / 2 - 250, 400, 500, 70);
            printText(renderer, "SIUUUUUUUUUUUUUUU", White, 70, SCREEN_WIDTH / 2 - 250, 500, 500, 70);
            //SDL_Delay(1000);
            playSound(Siu);
        }

    currentScore = 0;
    
    // 2 buttons
    SDL_Rect button[3][4];
    
    button[0][0] = { 50, 750, 400, 100 };
    button[0][1] = { 55, 755, 390, 90 };
    button[0][2] = { 60, 760, 380, 80 };
    button[0][3] = { 65, 765, 370, 70 };

    button[1][0] = { SCREEN_WIDTH - 450, 750, 400, 100 };
    button[1][1] = { SCREEN_WIDTH - 445, 755, 390, 90 };
    button[1][2] = { SCREEN_WIDTH - 440, 760, 380, 80 };
    button[1][3] = { SCREEN_WIDTH - 435, 765, 370, 70 };

    button[2][0] = { SCREEN_WIDTH / 2 - 200, 750, 400, 100 };
    button[2][1] = { SCREEN_WIDTH / 2 - 195, 755, 390, 90 };
    button[2][2] = { SCREEN_WIDTH / 2 - 190, 760, 380, 80 };
    button[2][3] = { SCREEN_WIDTH / 2 - 185, 765, 370, 70 };

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++)
        {
            if (j % 2 == 0) { SDL_SetRenderDrawColor(renderer, White.r, White.g, White.b, 255); }
            else { SDL_SetRenderDrawColor(renderer, Black.r, Black.g, Black.b, 255); }
            SDL_RenderFillRect(renderer, &button[i][j]);
        }

    const char* buttonText[3] = 
    {
        " << MENU",
        "PLAY AGAIN",
        "HIGH SCORES"
    };
    for(int i = 0; i < 3; i++) { printText(renderer, buttonText[i], White, 100, button[i][3].x + 10, button[i][3].y + 10, button[i][3].w - 20, button[i][3].h - 20); }

    SDL_RenderPresent(renderer);

    SDL_Event event;
    while (true)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    int b = -1;
                    for (int i = 0; i < 5; i++)
                    {
                        if (pointInRect({ event.button.x, event.button.y }, button[i][0])) { b = i; break; }
                    }
                    if (b >= 0) { playSound(buttonSound); }
                    if (b == 0) { menu(); return; }
                    if (b == 1) { mainCharacterInit(); game(); return; }
                    if (b == 2) { highScores(); return; }
                }
            }
            else if (event.type == SDL_QUIT)
            {
                playSound(buttonSound);
                SDL_Delay(1000);
                quitSDL(window, renderer);
                return;
            }
        }
    }
}

void pause()
{
    SDL_RenderCopy(renderer, Background, NULL, NULL);
    for (Character i : c) { i.draw(); }
    printText(renderer, "ON PAUSE", White, 120, SCREEN_WIDTH / 2 - 500, 50, 1000, 120);
    SDL_RenderPresent(renderer);

    SDL_Delay(1000);

    bool conti = false;
    SDL_Event pauseEvent;
    while (true)
    {
        if (conti == true) { break; }

        while (SDL_PollEvent(&pauseEvent))
        {
            switch (pauseEvent.type)
            {
            case SDL_KEYDOWN:
            {
                if (pauseEvent.key.keysym.scancode == SDL_SCANCODE_SPACE) { conti = true; }
                break;
            }
            case SDL_QUIT:
            {
                exit(0);
                break;
            }
            }
        }
    }
    for (int i = 3; i >= 1; i--)
    {
        string st = "GAME COUNTINUES IN " + to_string(i);
        const char * text = st.c_str();

        SDL_RenderCopy(renderer, Background, NULL, NULL);
        for (Character i : c) { i.draw(); }
        printText(renderer, text, White, 120, SCREEN_WIDTH / 2 - 500, 50, 1000, 120);
        SDL_RenderPresent(renderer);

        SDL_Delay(1000);
    }
    
    SDL_RenderCopy(renderer, Background, NULL, NULL);
    for (Character i : c) { i.draw(); }
    printText(renderer, "NOW", White, 120, SCREEN_WIDTH / 2 - 150, 50, 300, 120);
    SDL_Delay(500);
    SDL_RenderPresent(renderer);
}

void start()
{
    for (int i = 3; i >= 1; i--)
    {
        string st = "GAME STARTS IN " + to_string(i);
        const char* text = st.c_str();

        SDL_RenderCopy(renderer, Background, NULL, NULL);
        for (Character i : c) { i.draw(); }
        printText(renderer, text, White, 120, SCREEN_WIDTH / 2 - 500, 50, 1000, 120);
        SDL_RenderPresent(renderer);

        SDL_Delay(1000);
    }

    SDL_RenderCopy(renderer, Background, NULL, NULL);
    for (Character i : c) { i.draw(); }
    printText(renderer, "NOW", White, 120, SCREEN_WIDTH / 2 - 150, 50, 300, 120);
    SDL_Delay(500);
    SDL_RenderPresent(renderer);
}

void kill(int id)
{
    playSound(swordSliceSound);
    c.erase(c.begin() + id, c.begin() + id + 1);
}

int dist(Point u, Point v)
{
    return sqrt((u.x - v.x) * (u.x - v.x) + (u.y - v.y) * (u.y - v.y));
}

void mainCharacterInit()
{
    Character m(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 10, White);
    c.push_back(m);
}

void enemiesInit()
{
    int n = randLL(2, 5);
    for (int i = 1; i <= n; i++)
    {
        int x, y;
        again:
        x = randLL(0, SCREEN_WIDTH);
        y = randLL(0, SCREEN_HEIGHT);
        for (int j = 0; j < c.size(); j++) if (dist({x, y}, c[j].center) <= 300) { goto again; }

        Character enemy(x, y, randLL(2, 5), Black);

        c.push_back(enemy);
    }
}

void collideCheckandUpdate()
{
    if (c.size() <= 1) { return; }

    for(int u = 0; u < c.size() - 1; u++)
        for (int v = u + 1; v < c.size(); v++)
            {
                if (dist(c[u].center, c[v].center) > 300) { continue; }
                // Check if u can kill v
                bool vKilled = false;
                for(set<Point> &s : c[u].sword)
                    for (Point i : s)
                    {
                        auto it = c[v].body.find(i);
                        if (it != c[v].body.end()) 
                            {
                                kill(v);
                                vKilled = true;
                                goto skipV;
                            }
                    }  
                skipV:
                if (vKilled == true) 
                    { 
                        if (v == 0) { total(); return; }
                        currentScore++;
                        v--; 
                        continue; 
                    }
    
                // Check if v can kill u
                bool uKilled = false;
                for (set<Point> &s : c[v].sword)
                    for (Point i : s)
                    {
                        auto it = c[u].body.find(i);
                        if (it != c[u].body.end())
                        {
                            kill(u);
                            uKilled = true;
                            goto skipU;
                        }
                    }
                skipU:
                if (uKilled == true) 
                    { 
                        if (u == 0) { total(); return; }
                        currentScore++;
                        u--; 
                        continue; 
                    }

                // Check if swords collide
                for (int i = 0; i < c[u].sword.size(); i++)
                {
                    set<Point> s = c[u].sword[i];
                    if (s.empty()) { continue; }
                    for (Point p : s)
                        for (int j = 0; j < c[v].sword.size(); j++)
                        {
                            set<Point> t = c[v].sword[j];
                            if (t.empty()) { continue; }
                            auto it = t.find(p);
                            if (it != t.end())
                            {
                                playSound(swordCollideSound);
                                
                                // Remove sword
                                c[u].swordAngle[i] = -1e4;
                                c[v].swordAngle[j] = -1e4;
                            }
                        }
                }
            }
}

void game()
{
    start();

    SDL_Event event;
    bool keyPressed[SDL_NUM_SCANCODES] = { false };
    string Txt1, Txt2;

    while (true)
    {
        if (c.size() == 1) { enemiesInit(); }

        // Scene
        SDL_RenderCopy(renderer, Background, NULL, NULL);
        for(Character& i : c) 
            {
                i.rotate();
                i.draw(); 
            }

        Txt1 = "HIGHEST SCORE: " + to_string(max(currentScore, Score[0]));
        Txt2 = "YOUR SCORE: " + to_string(currentScore);
        printText(renderer, Txt1.c_str(), White, 70, SCREEN_WIDTH - 400, 10, 300, 70);
        printText(renderer, Txt2.c_str(), White, 70, SCREEN_WIDTH - 400, 80, 300, 70);
        SDL_RenderPresent(renderer);
        
        // Move
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
            {
                playSound(buttonSound);
                SDL_Delay(1000);
            
                saveLastGame();
                c.clear();
                return;
            }
            case SDL_KEYDOWN:
            {
                keyPressed[event.key.keysym.scancode] = true;

                if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) { pause(); }

                break;
            }
            case SDL_KEYUP:
            {
                keyPressed[event.key.keysym.scancode] = false;
                break;
            }
            }
        }

        if (keyPressed[SDL_SCANCODE_UP] && keyPressed[SDL_SCANCODE_RIGHT])
        {
            c[0].moveUp();
            c[0].moveRight();
        }
        else if (keyPressed[SDL_SCANCODE_UP] && keyPressed[SDL_SCANCODE_LEFT])
        {
            c[0].moveUp();
            c[0].moveLeft();
        }
        else if (keyPressed[SDL_SCANCODE_DOWN] && keyPressed[SDL_SCANCODE_RIGHT])
        {
            c[0].moveDown();
            c[0].moveRight();
        }
        else if (keyPressed[SDL_SCANCODE_DOWN] && keyPressed[SDL_SCANCODE_LEFT])
        {
            c[0].moveDown();
            c[0].moveLeft();
        }
        else if (keyPressed[SDL_SCANCODE_UP]) { c[0].moveUp(); }
        else if (keyPressed[SDL_SCANCODE_DOWN]) { c[0].moveDown(); }
        else if (keyPressed[SDL_SCANCODE_RIGHT]) { c[0].moveRight(); }
        else if (keyPressed[SDL_SCANCODE_LEFT]) { c[0].moveLeft(); }

        // Collide check
        collideCheckandUpdate();

        if (c.empty()) { return; }
    }
}

void menu()
{
    // Background
    SDL_Texture* Background = loadTexture("Resources/Images/Background.png", renderer);
    SDL_RenderCopy(renderer, Background, NULL, NULL);

    // Game name
    printText(renderer, "!!! RUN AWAY !!!", White, 120, SCREEN_WIDTH / 2 - 500, 50, 1000, 120);

    // 5 Buttons
    SDL_Rect button[5][4];
    button[0][0] = { SCREEN_WIDTH / 2 - 200, 240, 400, 100 };
    button[0][1] = { SCREEN_WIDTH / 2 - 195, 245, 390, 90 };
    button[0][2] = { SCREEN_WIDTH / 2 - 190, 250, 380, 80 };
    button[0][3] = { SCREEN_WIDTH / 2 - 185, 255, 370, 70 };
    for (int i = 1; i < 5; i++)
        {
            button[i][0] = {SCREEN_WIDTH / 2 - 200, button[i-1][0].y + 120, 400, 100};
            button[i][1] = {SCREEN_WIDTH / 2 - 195, button[i-1][1].y + 120, 390, 90};
            button[i][2] = {SCREEN_WIDTH / 2 - 190, button[i-1][2].y + 120, 380, 80};
            button[i][3] = {SCREEN_WIDTH / 2 - 185, button[i-1][3].y + 120, 370, 70};
        }

    for(int i = 0; i < 5; i++)
        for (int j = 0; j < 4; j++)
            {
                if (j % 2 == 0) { SDL_SetRenderDrawColor(renderer, White.r, White.g, White.b, 255); }
                else { SDL_SetRenderDrawColor(renderer, Black.r, Black.g, Black.b, 255); }
                SDL_RenderFillRect(renderer, &button[i][j]);
            }

    const char* buttonText[5] = 
    {
        "NEW GAME",
        "CONTINUE",
        "HIGH SCORES",
        "HELP",
        "EXIT"
    };
    for(int i = 0; i < 3; i++) { printText(renderer, buttonText[i], White, 100, button[i][3].x + 10, button[i][3].y + 10, button[i][3].w - 20, button[i][3].h - 20); }
    for(int i = 3; i < 5; i++) { printText(renderer, buttonText[i], White, 100, button[i][3].x + 70, button[i][3].y + 10, button[i][3].w - 150, button[i][3].h - 20); }

    SDL_RenderPresent(renderer);

    SDL_Event event;
    while (true)
        {
            while (SDL_PollEvent(&event)) 
                {
                    if (event.type == SDL_MOUSEBUTTONDOWN) 
                        {
                            if (event.button.button == SDL_BUTTON_LEFT) 
                                {
                                    int b = -1;
                                    for (int i = 0; i < 5; i++)
                                        {
                                            if (pointInRect({event.button.x, event.button.y}, button[i][0])) { b = i; break; }
                                        }
                                    if (b >= 0) { playSound(buttonSound); }
                                    if (b == 0) { mainCharacterInit(); game(); return; }
                                    if (b == 1) 
                                        { 
                                            if (loadLastGame() == true) { game(); return; }
                                        }
                                    if (b == 2) { highScores(); return; }
                                    if (b == 3) { help(); return; }
                                    if (b == 4) { SDL_Delay(1000); quitSDL(window, renderer); return; }
                                }
                        }
                    else if (event.type == SDL_QUIT) 
                        {
                            playSound(buttonSound);
                            SDL_Delay(1000);
                            quitSDL(window, renderer);
                            return;
                        }
            }
        }
}

void saveScore()
{
    ofstream out("Resources/High Scores.txt");
    
    for(int i = 0; i < 10; i++) { out << Score[i] << '\n'; }

    out.close();
}

void saveLastGame()
{
    ofstream out("Resources/Last Game.txt");
    
    if (c.empty()) { out << 0; }
    else
    {
        out << 1 << '\n';
        out << c.size() << '\n';
        for (int i = 0; i < c.size(); i++)
            {
                out << c[i].center.x << " " << c[i].center.y << '\n';
                out << c[i].swordCount << '\n';
                for(double angle : c[i].swordAngle) { out << angle << " "; }
                out << '\n';
                out << (int)c[i].color.r << " " << (int)c[i].color.g << " " << (int)c[i].color.b << '\n';
            }
        out << currentScore;
    }
    
    out.close();
}

int main(int argc, char* argv[])
{
    loadScore();

    menu();

    saveScore();

    quitSDL(window, renderer);

    return 0;
}