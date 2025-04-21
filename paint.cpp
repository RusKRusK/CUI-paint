#include <stdio.h>
#include <stdlib.h>
#include <termios.h>  //キー入力関係
#include <unistd.h>   //キー入力関係
#include <fcntl.h>    //キー入力関係
#include <stdbool.h>
#include <string.h>
#include <vector>

#define COLOR_BAR 8

struct cursor{
    int x;
    int y;
};

cursor poss;
int col = 0;
int dx[4] = {1,0,-1,0};
int dy[4] = {0,1,0,-1};
int c_bar[4] = {-16,16,-1,1};
int idcol;
bool pen = 0;
std::vector<std::vector<int>> canvas;
int canvas_h;
int canvas_w;

void init(void);
void drow(int);
void setBufferedInput(bool);
void fill(int,int);
void expo(void);
void expo2(void);
void save(void);
void load(void);
void move(int);
void resize(void);
void reverse(int);

void setBufferedInput(bool enable){
    static bool enabled = true;
    static struct termios old;
    struct termios new_;
    if (enable && !enabled){
        tcsetattr(STDIN_FILENO, TCSANOW, &old);
        enabled = true;
    }
    else if (!enable && enabled){
        tcgetattr(STDIN_FILENO, &new_);
        old = new_;
        new_.c_lflag &= (~ICANON & ~ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &new_);
        enabled = false;
    }
}

void init(void){
    system("clear");
    poss.x = 0;
    poss.y = 0;
    canvas.resize(canvas_h, std::vector<int>(canvas_w));
    for (int y = 0; y < canvas_h; y++){
        for (int x = 0; x < canvas_w; x++){
            canvas[y][x] = 0;
        }
    }
    drow(0);
    setBufferedInput(false);
}

void drow(int c){
    // system("clear");
    printf("\e[0;0H");
    if(pen)canvas[poss.y][poss.x]=col;
    for(int i=0;i<=canvas_w+1; i++)printf("■ ");putchar('\n');
    for (int y = 0; y < canvas_h; y++){
        printf("■ ");
        for (int x = 0; x < canvas_w; x++){
            printf("\e[48;5;%dm", canvas[y][x]);
            if(x == poss.x && y == poss.y)printf("\e[38;5;%dm%s",col,pen?"\e[38;5;255m● ":"■ ");
            else printf("　");
            printf("\e[0m");
        }
        printf("■ \n");
    }
    for(int i=0;i<=canvas_w+1; i++)printf("■ ");
    putchar('\n');
    for (int i = -COLOR_BAR; i < COLOR_BAR; i++)printf("\e[48;5;%dm\e[38;5;232m     \e[0m", (c + 256 + i) % 256);putchar('\n');
    for (int i = -COLOR_BAR; i < COLOR_BAR; i++)printf("\e[48;5;%dm\e[38;5;232m %03d \e[0m", (c + 256 + i) % 256, (c + 256 + i) % 256);putchar('\n');
    for (int i = -COLOR_BAR; i < COLOR_BAR; i++)printf("\e[48;5;%dm\e[38;5;232m     \e[0m", (c + 256 + i) % 256);
    putchar('\n');
    printf("%*s^\n", 5 * COLOR_BAR + 2, "");
    printf("矢印:色変更  w,a,s,d:カーソル移動  space:ドットを打つ  x:消しゴム  f:塗りつぶし  v:スポイト  r:連続打ち\nn:キャンバスをクリア  t:キャンバスをリサイズ  1,2,3,4:絵を移動  5:上下反転  6:左右反転\nz:カーソル表示/非表示  e:テキストとして出力  c:セーブ  l:ロード  q:終了\n");
    printf("\e[0J");
}

void fill(int y, int x){
    if(x<0||y<0||x>=canvas_w||y>=canvas_h||canvas[y][x]!=idcol)return;
    canvas[y][x]=col;
    for(int i=0;i<4;i++){
        fill(y+dy[i],x+dx[i]);
    }
}

void expo(void){
    int last_col = -1;
    printf("\n\n");
    printf("printf(\"");
    for(int y=0;y<canvas_h;y++){
        for(int x=0;x<canvas_w;x++){
            if(canvas[y][x]!=last_col)printf("\\e[48;5;%dm",canvas[y][x]);
            printf("　");
            last_col=canvas[y][x];
        }
        if(last_col){printf("\\e[0m");last_col=0;}
        printf("　\\n");
    }
    printf("\\e[0m\\n\");\n");
}

void expo2(void){
    char name[30];
    printf("配列の名前を入力してください.");
    scanf("%s",name);
    printf("\nint %s[%d][%d] = {",name,canvas_h,canvas_w);
    for(int y=0;y<canvas_h;y++){
        printf("{");
        for(int x=0;x<canvas_w;x++){
            printf("%d%s",canvas[y][x],x==canvas_w-1?"":",");
        }
        printf("%s\n",y==canvas_h-1?"}};":"},");
    }
}

void save(void){
    FILE* fp;
    char name[30];
    puts("ファイル名を入力してください.");
    scanf("%s",name);
    if(!strcmp(name, "paint.cpp")||!strcmp(name, "paint")){printf("使用できない名前です.\n");return;}
    fp = fopen(name,"w");
    fprintf(fp,"%d\t%d\n",canvas_h,canvas_w);
    for(int y=0;y<canvas_h;y++){
        for(int x=0;x<canvas_w;x++){
            fprintf(fp,"%03d\t",canvas[y][x]);
        }
    }
    printf("ファイル名 \"%s\" で保存しました！\n",name);
    fclose(fp);
}

void load(void){
    FILE* fp;
    char name[30];
    char buf[256];
    char num[20];
    puts("ロードするファイル名を入力してください.");
    system("ls");
    scanf("%s",name);
    if(!strcmp(name, "paint.cpp")||!strcmp(name, "paint")){printf("使用できない名前です.\n");return;}
    fp = fopen(name,"r");
    if(fp == NULL){printf("ファイルが存在しないもしくは開けません.\n");return;}
    if(fgets(buf, 256, fp) == NULL)return;
    sscanf(buf,"%d\t%d\n",&canvas_h,&canvas_w);
    canvas.resize(canvas_h);
    for(int i=0;i<canvas_h;i++)canvas[i].resize(canvas_w);
    for(int y=0;y<canvas_h;y++){
        for(int x=0;x<canvas_w;x++){
            if(fgets(num, 5, fp) == NULL)break;
            sscanf(num,"%d\t",&canvas[y][x]);
        }
    }
    drow(col);
    fclose(fp);
    system("clear");
}

void move(int ar){
    std::vector<std::vector<int>> buf;
    buf.resize(canvas_h);
    for(int i=0;i<canvas_h;i++)buf[i].resize(canvas_w);
    for (int y = 0; y < canvas_h; y++){
        for (int x = 0; x < canvas_w; x++){
            buf[y][x] = 0;
        }
    }
    for(int y=0;y<canvas_h;y++){
        for(int x=0;x<canvas_w;x++){
            if(y + dy[ar]<0||y + dy[ar]>=canvas_h||x + dx[ar]<0||x + dx[ar]>=canvas_w)continue;
            buf[y + dy[ar]][x + dx[ar]] = canvas[y][x];
        }
    }
    for(int y=0;y<canvas_h;y++){
        for(int x=0;x<canvas_w;x++){
            canvas[y][x] = buf[y][x];
        }
    }
}

void resize(void){
    int w,h;
    puts("縦の長さ，横の長さを入力してください.");
    scanf("%d%d",&h,&w);
    for(int i=0;i<canvas_h;i++)canvas[i].resize(w);
    canvas.resize(h, std::vector<int>(w));
    canvas_h = h;
    canvas_w = w;
    system("clear");
}

void reverse(int mode){
    int buf[canvas_h][canvas_w]={0};
    for(int y=0;y<canvas_h;y++){
        for(int x=0;x<canvas_w;x++){
            switch(mode){
                case 0:
                    buf[y][x]=canvas[canvas_h-y-1][x];
                    break;
                case 1:
                    buf[y][x]=canvas[y][canvas_w-x-1];
                    break;
                default:break;
            }
        }
    }
    for(int y=0;y<canvas_h;y++){
        for(int x=0;x<canvas_w;x++){
            switch(mode){
                case 0:
                case 1:
                    canvas[y][x] = buf[y][x];
                default:break;
            }
        }
    }
}

int main(int argc, char *argv[]){
    if(argc!=3){
        printf("./paint <キャンバスの高さ> <キャンバスの横幅> のように入力してください.\n");
        return 0;
    }
    canvas_h = atoi(argv[1]);
    canvas_w = atoi(argv[2]);
    char c;
    int arrow=0;
    init();
    while (1){
        c=getchar();
        switch(c){
            case 'q':
                puts("終了しますか?(y/n)");
                switch(getchar()){
                    case 'y':system("stty sane");return 0;
                    case 'n':
                    default:drow(col);break;
                }
            case 'w':arrow++;
            case 'a':arrow++;
            case 's':arrow++;
            case 'd':
                if(poss.x + dx[arrow] >= 0 && poss.x + dx[arrow] < canvas_w && poss.y + dy[arrow] >= 0 && poss.y + dy[arrow] < canvas_h){
                    poss.x = poss.x + dx[arrow];
                    poss.y = poss.y + dy[arrow];
                }
                arrow = 0;
                drow(col);
                break;
            case 67:arrow++;
            case 68:arrow++;
            case 66:arrow++;
            case 65:
                //col = (col + 256 + c_bar[arrow]) % 256;
                col = (col + c_bar[arrow]) & 255;
                drow(col);
                arrow = 0;
                break;
            case 32:
                canvas[poss.y][poss.x] = col;
                drow(col);
                break;
            case 'x':
                canvas[poss.y][poss.x] = 0;
                drow(col);
                break;
            case 'z':
                if(poss.x!=-1&&poss.y!=-1){
                    poss.x=-1;poss.y=-1;drow(col);
                }
                else{
                    poss.x=0;poss.y=0;drow(col);
                }
                break;
            case 'f':
                if(canvas[poss.y][poss.x]!=col){
                    idcol = canvas[poss.y][poss.x];
                    fill(poss.y,poss.x);
                    drow(col);
                }
                break;
            case 'v':
                col = canvas[poss.y][poss.x];
                drow(col);
                break;
            case 'e':
                drow(col);
                puts("print文での出力:1  配列としての出力:2");
                switch(getchar()){
                    case '1':expo();break;
                    case '2':expo2();break;
                    default:break;
                }
                break;
            case 'r':
                pen = !pen;
                drow(col);
                break;
            case 'c':
                drow(col);
                save();
                break;
            case 'l':
                drow(col);
                load();
                drow(col);
                break;
            case '3':arrow++;
            case '1':arrow++;
            case '2':arrow++;
            case '4':
                move(arrow);
                drow(col);
                arrow = 0;
                break;
            case 'n':
                puts("キャンバスをクリアしますか?(y/n)");
                switch(getchar()){
                    case 'y':
                    for(int y=0;y<canvas_h;y++){
                        for(int x=0;x<canvas_w;x++){
                            canvas[y][x]=0;
                        }
                    }
                    case 'n':
                    default:drow(col);break;
                }
                drow(col);
                break;
            case 't':
                resize();
                drow(col);
                break;
            case '5':
                reverse(0);
                drow(col);
                break;
            case '6':
                reverse(1);
                drow(col);
                break;
            case '0':
                system("clear");
                drow(col);
                break;
        }
    }
}