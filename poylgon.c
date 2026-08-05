#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <windows.h>
#define RES_X 128 //resolution r[0]
#define RES_Y 64 //resolution r[1]; right-click on the window title, select properties and set the proper size
#define CHAR_ASPECT 2 //aspect ratio of font(height/width)

#define M_2PI M_PI * 2
#define ANG_ACCEL M_2PI * 0x1p-6
#define VELOCITY 0x1p-4
double rot[2][2];
double v[3] = { 0,0,0 };

struct
{
	double z[RES_X * RES_Y];
	short ch[RES_X * RES_Y]; //screen character 
}buff;

short char_set[] = { '-','+','+','-','+','-','-','+' };

typedef struct
{
	double a[3][3];//three vertexes of triangle
	double b[3][3];//b[0]:normal vector; b[1]:vector from a[0] to a[1]; b[2]:vector from a[0] to a[2]; 
	double c[2][2];
	short ch;
}TRI;

#define TRI_COUNT 512
TRI tri[TRI_COUNT];

double min3(double* v)
{
	double w;
	if (v[0] <= v[1])
		w = v[0];
	else
		w = v[1];
	if (w <= v[2])
		return w;
	else
		return v[2];
}

double max3(double* v)
{
	double w;
	if (v[0] >= v[1])
		w = v[0];
	else
		w = v[1];
	if (w >= v[2])
		return w;
	else
		return v[2];
}

int init()
{
	rot[0][0] = cos(ANG_ACCEL);
	rot[0][1] = -sin(ANG_ACCEL);
	rot[1][0] = sin(ANG_ACCEL);
	rot[1][1] = cos(ANG_ACCEL);
	for (int n = 0; n < TRI_COUNT; n += 8)
	{
		for (int m = 0; m < 8; ++m)
		{
			TRI* t = &tri[n + m];
			for (int i = 0; i < 3; ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					if (i == j)
						t->a[i][j] = m & (1 << i) ? 1 : -1;
					else
						t->a[i][j] = 0;
					t->a[i][j] += (((n >> 3) >> (j * 2)) & 3) * 2;
				}
			}
			t->ch = char_set[m];
		}
	}
	for (int n = 0; n < TRI_COUNT; ++n)
	{
		TRI* t = &tri[n];
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				t->a[i][j] -= 3;
			}
			//t->a[i][2] +=8;
		}
	}
	for (int n = 0; n < TRI_COUNT; ++n)
	{
		TRI* t = &tri[n];
		for (int i = 1; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				t->b[i][j] = t->a[i][j] - t->a[0][j];
			}
		}
		double len = 0;
		for (int j = 0; j < 3; ++j)
		{
			t->b[0][j] = t->b[1][(j + 1) % 3] * t->b[2][(j + 2) % 3] - t->b[1][(j + 2) % 3] * t->b[2][(j + 1) % 3];
			len += t->b[0][j] * t->b[0][j];
		}
		len = sqrt(len);
		for (int j = 0; j < 3; ++j)
		{
			t->b[0][j] /= len;
		}
	}
	return 0;
}

double trace(double dist, double* v, TRI* tri)
{
	double ba = 0;
	for (int j = 0; j < 3; ++j)
	{
		ba += tri->a[0][j] * tri->b[0][j];
	}
	double bv = 0;
	for (int j = 0; j < 3; ++j)
	{
		bv += v[j] * tri->b[0][j];
	}
	if (bv == 0)
	{
		return  dist;
	}
	double ratio = ba / bv;
	if (ratio > dist || ratio < 0)
	{
		return  dist;
	}
	double p[2] = { 0 };
	for (int j = 0; j < 2; ++j)
	{
		p[j] = ratio * v[j + 1] - tri->a[0][j + 1];
	}
	double d = tri->b[1][1] * tri->b[2][2] - tri->b[2][1] * tri->b[1][2];
	if (d == 0)
		return dist;
	double s = tri->b[2][2] * p[0] - tri->b[2][1] * p[1];
	s /= d;
	double t = -tri->b[1][2] * p[0] + tri->b[1][1] * p[1];
	t /= d;
	if (s > 0 && t > 0 && s + t < 1)
	{
		return ratio;
	}
	else
	{
		return dist;
	}
}

int draw()
{
	//rotation
	int keylist1[8] = { 'S','D','F','G','H','J','K','L' };
	int keystate[8];
	for (int i = 0; i < 8; ++i)
		keystate[i] = GetKeyState(keylist1[i]);
	for (int n = 0; n < TRI_COUNT; ++n)
	{
		TRI* t = &tri[n];
		double* v_list[6] = { t->a[0],t->a[1],t->a[2],t->b[0],t->b[1],t->b[2] };
		for (int i = 0; i < 6; ++i)
			if (keystate[i] & 0x8000)
				for (int j = 0; j < 6; ++j)
				{
					double r_tmp[2] = { 0 };
					for (int k = 0; k < 2; ++k)
						for (int l = 0; l < 2; ++l)
							if (i % 2 != 0)
								r_tmp[k] += rot[l][k] * v_list[j][(l + i / 2) % 3];
							else
								r_tmp[k] += rot[k][l] * v_list[j][(l + i / 2) % 3];
					for (int k = 0; k < 2; ++k)
						v_list[j][(k + i / 2) % 3] = r_tmp[k];
				}
		for (int i = 6; i < 8; ++i)
			if (keystate[i] & 0x8000)
				for (int j = 0; j < 3; ++j)
					if (i % 2)
						v_list[j][2] -= VELOCITY;
					else
						v_list[j][2] += VELOCITY;
		double c[2][3] = { {-1,1,1},{-1,-1,1} };
		if (t->a[0][2] > 0 && t->a[1][2] > 0 && t->a[2][2] > 0)
			for (int i = 0; i < 3; ++i)
			{
				for (int j = 0; j < 2; ++j)
				{
					c[j][i] = t->a[i][j] / t->a[i][2];
				}
			}
		for (int j = 0; j < 2; ++j)
		{
			t->c[j][0] = max(min3(c[j]), -1);
			t->c[j][1] = min(max3(c[j]), 1);
		}
	}
	//ray tracing
	for (int i = 0; i < RES_X * RES_Y; ++i)
	{
		buff.ch[i] = ' ';
		buff.z[i] = 256;
	}
	for (int n = 0; n < TRI_COUNT; ++n)
	{
		TRI*t = &tri[n];
		if (t->a[0][2] > 0 || t->a[1][2] > 0 || t->a[2][2] > 0)
		{
			double ray[3];
			ray[2] = 1; //printf("%f\n", ray[2]);
			//X方向の走査
			for (int i = (int)ceil(((t->c[0][0] + 1) * RES_X - 1) / 2); i < floor(((t->c[0][1] + 1) * RES_X - 1) / 2); ++i)
			{
				ray[0] = (i * 2 - RES_X + 1) / (double)RES_X; //printf("%f\t", ray[0]);
				//Y方向の走査
				for (int j = (int)ceil(((t->c[1][0] + 1) * RES_Y - 1) / 2); j < floor(((t->c[1][1] + 1) * RES_Y - 1) / 2); ++j)
				{
					ray[1] = (j * 2 - RES_Y + 1) / (double)RES_Y; //printf("%f\t", ray[1]);
					double d = trace(buff.z[i + j * RES_X], ray, t);
					if (d < buff.z[i + j * RES_X])
					{
						buff.ch[i + j * RES_X] = t->ch;
						buff.z[i + j * RES_X] = d;
					}
				}
			}
		}
	}
	return 0;
}

void display()//main display function; windows-specific
{
	HANDLE hand = GetStdHandle(STD_OUTPUT_HANDLE); //handle to console window
	DWORD written; //helper flag, unused
	WriteConsole(hand, buff.ch, RES_X * RES_Y, &written, NULL); //write the character contents of buff[]
}

int main()
{
	if (init())
		return -1;
	while (1)
	{
		if (GetKeyState(VK_ESCAPE) & 0x8000)//push ESCAPE to shutdown the app
			return 0;
		if (draw())
			return -1;
		//return 0;
		display();
		Sleep(25);
	}
}