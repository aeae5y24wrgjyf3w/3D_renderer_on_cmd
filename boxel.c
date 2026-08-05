#include <math.h>
#include <windows.h>
#define RES_X 128 //resolution r[0]
#define RES_Y 64 //resolution r[1]; right-click on the window title, select properties and set the proper size
#define CHAR_ASPECT 2 //aspect ratio of character
#define ANG_ACCEL 0x1p-12 //change in angle per frame
#define ACCEL 0x1p-12 //velocity change per frame
#define RANGE (1 << 6) //visible range
#define SIZE (1 << 3) //world size
#define LEVEL 1//cahnging objects' shape
#define MAP_B //you can swith the map by canging this

short char_buff[RES_X * RES_Y]; //screen character buffer
short char_set[3] = { '-','+',':' };

unsigned int block[SIZE][SIZE] = { 0 }; //map

double player[3] = { 0,0,0 }; //player's location
double v[3] = { 0,0,0 }; //player's location
double eye[3][3] = { {1,0,0},{0,1,0},{0,0,1} }; //player's cordinates
double rot[3][3] = { {1,0,0},{0,1,0},{0,0,1} }; //player's lotation
double rot_rot[3][3]; //rotation of player's rotation

void init() //initialize the map
{
	rot_rot[0][0] = cos(ANG_ACCEL);
	rot_rot[0][1] = -sin(ANG_ACCEL);
	rot_rot[0][2] = 0.0;

	rot_rot[1][0] = sin(ANG_ACCEL);
	rot_rot[1][1] = cos(ANG_ACCEL);
	rot_rot[1][2] = 0.0;

	rot_rot[2][0] = 0.0;
	rot_rot[2][1] = 0.0;
	rot_rot[2][2] = 1.0;
	for (int i = 0; i < SIZE; ++i)
		for (int j = 0; j < SIZE; ++j)
			for (int k = 0; k < SIZE; ++k)
#if defined MAP_A //cubes
				if (i < LEVEL && j < LEVEL && k < LEVEL)
					block[i][j] |= 1 << k;
#elif defined MAP_B //grid
				if (i < LEVEL && j < LEVEL
					|| j < LEVEL && k < LEVEL
					|| k < LEVEL && i < LEVEL)
					block[i][j] |= 1 << k;
#elif defined MAP_C //angled planes
				if (i + j + k == SIZE - 1 || i + j + k == SIZE * 2 - 1)
					block[i][j] |= 1 << k;
#elif defined MAP_D //grid 2
				if (i & 2 && j & 2
					|| j & 4 && k & 4
					|| k & 1 && i & 1)
					block[i][j] |= 1 << k;
#elif defined MAP_E //angled planes 2
				if (i / 2 + j + k == SIZE / 2 - 1
					|| i / 2 + j + k == SIZE - 1
					|| i / 2 + j + k == SIZE * 3 / 2 - 1
					|| i / 2 + j + k == SIZE * 2 - 1)
					block[i][j] |= 1 << k;
#endif
}

int trace(double* ray,int* cube) //identify the block given vector ("ray") is aiming at6
{
	int d;
	for (int j = 0; j < 3; ++j)
	{
		cube[j] = floor(player[j]);
		if (cube[j] == player[j])
		{
			d = j;
			if (ray[j] < 0)
				--cube[j];
		}
	}
	while (1)
	{
		if (block[cube[0] & (SIZE - 1)][cube[1] & (SIZE - 1)] & 1 << (cube[2] & (SIZE - 1)))
			return d;
		double rect[3];
		for (int j = 0; j < 3; ++j)
		{
			rect[j] = (double)cube[j] + (ray[j] >= 0) - player[j];
			if (rect[j] < -RANGE || rect[j] >= RANGE)
				return -1;
			rect[j] = ray[j] / rect[j];
		}
		if (rect[1] > rect[0])
			if (rect[2] > rect[1])
				d = 2;
			else
				d = 1;
		else
			if (rect[0] > rect[2])
				d = 0;
			else
				d = 2;
		cube[d] += (ray[d] >= 0) ? 1 : -1;
	}
}

void draw(void)//determine
{
	//determine your angular velocity
	int keylist1[6] = { 'S','D','F','G','H','J' };
	for (int i = 0; i < 6; ++i)
		if (GetKeyState(keylist1[i]) & 0x8000)
			for (int j = 0; j < 3; ++j)
			{
				double r_tmp[3] = { 0 };
				for (int k = 0; k < 3; ++k)
					for (int l = 0; l < 3; ++l)
						if (i % 2)
							r_tmp[k] += rot_rot[(l + i / 2) % 3][(k + i / 2) % 3] * rot[j][l];
						else
							r_tmp[k] += rot_rot[(k + i / 2) % 3][(l + i / 2) % 3] * rot[j][l];
				for (int k = 0; k < 3; ++k)
					rot[j][k] = r_tmp[k];
			}

	//determine your direction
	for (int j = 0; j < 3; ++j)
	{
		double r_tmp[3] = { 0 };
		for (int k = 0; k < 3; ++k)
			for (int l = 0; l < 3; ++l)
				r_tmp[k] += rot[l][k] * eye[l][j];
		for (int k = 0; k < 3; ++k)
			eye[k][j] = r_tmp[k];
	}

	//determine your velocity
	int keylist2[6] = { 0,0,0,0,'L','K' };
	for (int i = 0; i < 6; ++i)
		if (GetKeyState(keylist2[i]) & 0x8000)
			for (int j = 0; j < 3; ++j)
				if (i % 2)
					v[j] -= eye[i / 2][j] * ACCEL;
				else
					v[j] += eye[i / 2][j] * ACCEL;

	//bounce off the wall
	int d1;
	int cube1[3];
	double depth = 1;
	double player_tmp[3];
	while (1)
	{
		d1 = trace(v, cube1);
		for (int j = 0; j < 3; ++j)
			player_tmp[j] = player[j] + v[j] * depth;
		if (d1 < 0 || v[d1] == 0)
			break;
		else if (floor(player_tmp[d1]) != cube1[d1])
			break;
		depth = player_tmp[d1] - floor(player_tmp[d1]) - (v[d1] < 0);
		depth /= v[d1];
		for (int j = 0; j < 3; ++j)
			if (j == d1)
				player[j] = floor(player_tmp[d1]) + (v[d1] < 0);
			else
				player[j] = player_tmp[j] - v[j] * depth;
		v[d1] = -0.5 * v[d1];
	}

	//wrapping player's location
	for (int j = 0; j < 3; ++j)
	{
		player[j] = player_tmp[j];
		while (player[j] >= SIZE)
			player[j] -= SIZE;
		while (player[j] < 0)
			player[j] += SIZE;
	}

	//ray tracing
	for (int i=0;i< RES_X * RES_Y;++i)
	{
		double x = i % RES_X - RES_X / 2 + 0.5;
		double y = i / RES_X - RES_Y / 2 + 0.5;
		double ray[3];
		for (int j = 0; j < 3; ++j)
			ray[j] = eye[0][j] * x + eye[1][j] * y * CHAR_ASPECT + eye[2][j] * RES_X / 2;
		int cube[3];
		int d= trace(ray, cube);
		if (d >= 0)
			char_buff[i] = char_set[d];
		else
			char_buff[i] = ' ';
	}
}

void display()//main display function; widows-specific
{
	HANDLE hand = GetStdHandle(STD_OUTPUT_HANDLE); //handle to console window
	DWORD written; //helper flag, unused
	WriteConsole(hand, char_buff, RES_X * RES_Y, &written, NULL); //write the character contents of buff[]
}

int main()
{
	init();
	while (1)
	{
		if (GetKeyState(VK_ESCAPE) & 0x8000)//push ESCAPE to shutdown the app
			return 0;
		draw();
		display();
		Sleep(25);
	}
}