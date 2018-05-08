#include<iostream>
#include<Windows.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include<conio.h>
#include<thread>
#include<fstream>
using namespace std;

void printL(int, int);
void printT(int, int);

void FixConsoleWindow() {
	HWND consoleWindow = GetConsoleWindow();
	LONG style = GetWindowLong(consoleWindow, GWL_STYLE);
	style = style & ~(WS_MAXIMIZEBOX)& ~(WS_THICKFRAME);
	SetWindowLong(consoleWindow, GWL_STYLE, style);
}

void GotoXY(int x, int y) {
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

//Hằng số
#define MAX_CAR 17
#define MAX_CAR_LENGTH 6
#define MAX_SPEED 3
//Biến toàn cục
POINT** X; //Mảng chứa MAX_CAR xe
POINT Y; // Đại diện người qua đường
int cnt = 0;//Biến hỗ trợ trong quá trình tăng tốc độ xe di chuyển
int MOVING;//Biến xác định hướng di chuyển của người
int SPEED;// Tốc độ xe chạy (xem như level)
int HEIGH_CONSOLE = 20, WIDTH_CONSOLE = 50;// Độ rộng và độ cao của màn hình console
bool STATE; // Trạng thái sống/chết của người qua đường
int ViTriAnToan[48] = { 0 };//vi tri cua nguoi sau khi qua duong thanh cong
int widthL = WIDTH_CONSOLE + 10, heightL = 5;
int widthT = WIDTH_CONSOLE + 10, heightT = 7;
char ch[10], ch1[] = ".txt";

//Hàm khởi tạo dữ liệu mặc định ban đầu
void ResetData() {
	MOVING = 'D'; // Ban đầu cho người di chuyển sang phải
	SPEED = 1; // Tốc độ lúc đầu
	Y = { 18, 19 }; // Vị trí lúc đầu của người
	// Tạo mảng xe chạy
	if (X == NULL) {
		X = new POINT*[MAX_CAR];
		for (int i = 0; i < MAX_CAR; i++)
			X[i] = new POINT[MAX_CAR_LENGTH];
		for (int i = 0; i < MAX_CAR; i++)
		{
			int temp = (rand() % (WIDTH_CONSOLE - MAX_CAR_LENGTH)) + 1;
			for (int j = 0; j < MAX_CAR_LENGTH; j++)
			{
				X[i][j].x = temp + j;
				X[i][j].y = 2 + i;
			}
		}
	}
	for (int i = 0; i < 48; i++){
		ViTriAnToan[i] = 0;
	}
}

void DrawBoard(int x, int y, int width, int height, int curPosX = 0, int curPosY = 0)
{

	GotoXY(x, y); cout << 'X';
	for (int i = 1; i < width; i++)cout << 'X';
	cout << 'X';
	GotoXY(x, height + y); cout << 'X';
	for (int i = 1; i < width; i++)cout << 'X';
	cout << 'X';
	for (int i = y + 1; i < height + y; i++)
	{
		GotoXY(x, i); cout << 'X';
		GotoXY(x + width, i); cout << 'X';
	}
	GotoXY(curPosX, curPosY);
}


void StartGame() {
	system("cls");
	ResetData(); // Khởi tạo dữ liệu gốc
	DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE); // Vẽ màn hình game
	printL(widthL, heightL);
	printT(widthT, heightT);
	STATE = true;//Bắt đầu cho Thread chạy
}

//Hàm dọn dẹp tài nguyên
void GabageCollect()
{
	for (int i = 0; i < MAX_CAR; i++)
	{
		delete[] X[i];
	}
	delete[] X;
}

//Hàm thoát game
void ExitGame(HANDLE t) {
	system("cls");
	TerminateThread(t, 0);
	GabageCollect();
}

//Hàm dừng game
void PauseGame(HANDLE t) {
	SuspendThread(t);
}

//Hàm xử lý khi người đụng xe
void ProcessDead() {
	STATE = 0;
	GotoXY(0, HEIGH_CONSOLE + 2);
	printf("Dead, type y to continue or anykey to exit");
}

//Hàm xử lý khi người băng qua đường thành công
void ProcessFinish(POINT& p) {
	if (ViTriAnToan[p.x] == 0){   //xu ly nguoi cham nguoi
		ViTriAnToan[p.x] = 1;
	}
	else{
		ProcessDead();
		printf("kakaka");
	}
	SPEED == MAX_SPEED ? SPEED = 1 : SPEED++;
	p = { 18, 19 }; // Vị trí lúc đầu của người
	MOVING = 'D'; // Ban đầu cho người di chuyển sang phải
}

//Hàm vẽ các toa xe
void DrawCars(char* s)
{
	for (int i = 0; i < MAX_CAR; i++) {
		for (int j = 0; j < MAX_CAR_LENGTH; j++)
		{
			GotoXY(X[i][j].x, X[i][j].y);
			printf("=");
		}
	}
}

//Hàm vẽ người qua đường
void DrawSticker(const POINT& p, char* s) {
	GotoXY(p.x, p.y);
	printf(s);
}

//Hàm kiểm tra xem người qua đường có đụng xe không
bool IsImpact(const POINT& p, int d)
{
	if (d == 1 || d == HEIGH_CONSOLE - 1)return false;
	for (int i = 0; i < MAX_CAR_LENGTH; i++)
	{
		if (p.x == X[d - 2][i].x && p.y == X[d - 2][i].y) {

			return true;
		}
	}
	return false;
}

void MoveCars() {
	for (int i = 1; i < MAX_CAR; i += 2)
	{
		cnt = 0;
		do {
			cnt++;
			for (int j = 0; j < MAX_CAR_LENGTH - 1; j++) {
				X[i][j] = X[i][j + 1];
			}
			X[i][MAX_CAR_LENGTH - 1].x + 1 == WIDTH_CONSOLE ? X[i][MAX_CAR_LENGTH -
				1].x = 1 : X[i][MAX_CAR_LENGTH - 1].x++; // Kiểm tra xem xe có đụng màn hình không
		} while (cnt < SPEED);
	}
	for (int i = 0; i < MAX_CAR; i += 2)
	{
		cnt = 0;
		do {
			cnt++;
			for (int j = MAX_CAR_LENGTH - 1; j > 0; j--)
			{
				X[i][j] = X[i][j - 1];
			}
			X[i][0].x - 1 == 0 ? X[i][0].x = WIDTH_CONSOLE - 1 : X[i][0].x--;// Kiểm tra xem xe có đụng màn hình không
		} while (cnt < SPEED);
	}
}

// Hàm xóa xe (xóa có nghĩa là không vẽ)
void EraseCars()
{
	for (int i = 0; i < MAX_CAR; i += 2) {
		cnt = 0;
		do {
			GotoXY(X[i][MAX_CAR_LENGTH - 1 - cnt].x, X[i][MAX_CAR_LENGTH - 1 - cnt].y);
			printf(" ");
			cnt++;

		} while (cnt < SPEED);
	}
	for (int i = 1; i < MAX_CAR; i += 2) {
		cnt = 0;
		do {
			GotoXY(X[i][0 + cnt].x, X[i][0 + cnt].y);
			printf(" ");
			cnt++;
		} while (cnt < SPEED);
	}
}

void MoveRight() {
	if (Y.x < WIDTH_CONSOLE - 1)
	{
		DrawSticker(Y, " ");
		Y.x++;
		DrawSticker(Y, "Y");
	}
}
void MoveLeft() {
	if (Y.x > 1) {
		DrawSticker(Y, " ");
		Y.x--;
		DrawSticker(Y, "Y");
	}
}

void MoveDown() {
	if (Y.y < HEIGH_CONSOLE - 1)
	{
		DrawSticker(Y, " ");
		Y.y++;
		DrawSticker(Y, "Y");
	}
}

void MoveUp() {
	if (Y.y > 1) {
		DrawSticker(Y, " ");
		Y.y--;
		DrawSticker(Y, "Y");

	}

}

void printT(int width, int height){
	GotoXY(width, height);
	cout << "Load Game:";
}

void printL(int width, int height){
	GotoXY(width, height);
	cout << "Save Game:";
}

void Save(){
	strcat(ch, ch1);
	ofstream fp(ch, ios::trunc);
	for (int i = 0; i < 48; i++){
		fp << ViTriAnToan[i];
	}
	fp << " ";
	fp << SPEED;
	fp.close();
}


void Load(){
	char chr;
	strcat(ch, ch1);
	ifstream fp(ch);
	for (int i = 0; i < 48; i++){
		fp.get(chr);
		if (chr == '1'){
			Y = { i, 1 };
			DrawSticker(Y, "Y");
			Y = { 18, 19 };
			DrawSticker(Y, "Y");
		}
	}
	fp.get(chr); fp.get(chr);
	SPEED = (int)chr - 49;
	fp.close();
}

void Begin(){
	int j = 0;
	while (1){
		GotoXY(40, 17);
		printf("Mini Game ");
		for (int i = 0; i < 10; i++){
			if (i != j){
				GotoXY(40 + i, 17);
				printf(" ");
			}
			if (i == 9)j++;
		}
		Sleep(100);
		if (j == 9)j = 0;
	}
}

void SubThread()
{
	int dem = 0;
	while (1) {
		if (STATE) //Nếu người vẫn còn sống
		{
			switch (MOVING) //Kiểm tra biến moving
			{
			case 'A':
				MoveLeft();
				break;
			case 'D':
				MoveRight();
				break;
			case 'W':
				MoveUp();
				break;
			case 'S':
				MoveDown();
				break;
			}
			MOVING = ' ';// Tạm khóa không cho di chuyển, chờ nhận phím từ hàm main
			if (dem < 50){
				EraseCars();
				MoveCars();
				DrawCars(".");
			}
			if (dem>70){
				dem = 0;
			}
			if (IsImpact(Y, Y.y))
			{
				DrawSticker(Y, "X");
				ProcessDead(); // Kiểm tra xe có đụng không
			}
			if (Y.y == 1){
				ProcessFinish(Y); // Kiểm tra xem về đích chưa
			}
			dem++;
			Sleep(150);//Hàm ngủ theo tốc độ SPEED

		}

	}
}

void main()
{
	int temp;
	FixConsoleWindow();
	srand(time(NULL));
	temp = toupper(getch());
	if (temp == 'T'){
		Begin();
	}
	else{
		StartGame();
	}
	thread t1(SubThread);
	while (1)
	{
		temp = toupper(getch());
		if (STATE == 1)
		{
			if (temp == 27) {
				ExitGame(t1.native_handle());
				exit(0);

			}
			else if (temp == 'P') {
				PauseGame(t1.native_handle());

			}
			else if (temp == 'L'){
				PauseGame(t1.native_handle());
				GotoXY(widthL + 11, heightL);
				cin >> ch;
				Save();
			}
			else if (temp == 'T'){
				StartGame();
				PauseGame(t1.native_handle());
				GotoXY(widthT + 11, heightT);
				cin >> ch;
				Load();
				temp = 'S';
			}
			else {
				ResumeThread((HANDLE)t1.native_handle());
				if (temp == 'D' || temp == 'A' || temp == 'W' || temp == 'S')
				{
					MOVING = temp;
				}
			}

		}
		else
		{
			if (temp == 'Y') StartGame();
			else {
				ExitGame(t1.native_handle());
				exit(0);
			}
		}
	}
}