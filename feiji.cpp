//小游戏
//easyx

#include<iostream>
#include<graphics.h>//easyx
#include<vector>//建立容器管理敌机
#include<conio.h>
using namespace std;
//一个开始界面
constexpr auto swidth = 600;
constexpr auto sheight = 1100;

bool PointInRect(int x, int y, RECT& r)
{
	return(r.left <= x && x <= r.right && r.top <= y && y <= r.bottom);
}

bool RectDuangRect(RECT& r1, RECT& r2)
{
	RECT r;
	r.left = r1.left - (r2.right - r2.left);
	r.right = r1.right;
	r.top = r1.top - (r2.bottom - r2.top);
	r.bottom = r1.bottom;

	return (r.left < r2.left&& r2.left <= r.right && r.top <= r2.top && r2.top <= r.bottom);

}
//开始界面
void Welcome()
{
	LPCTSTR title = _T(" 飞机大战");
	LPCTSTR tplay = _T("开始游戏");
	LPCTSTR texit = _T("退出游戏");

	RECT tplayr, texitr;//文字矩形框的范围
	BeginBatchDraw();//减少频闪
	setbkcolor(WHITE);//背景颜色
	cleardevice();//初始化
	settextstyle(60, 0, _T("黑体"));
	settextcolor(BLACK);
	outtextxy(swidth / 2 - textwidth(title) / 2, sheight / 5, title);

	settextstyle(40, 0, _T("黑体"));
	tplayr.left = swidth / 2 - textwidth(tplay) / 2;
	tplayr.right = tplayr.left + textwidth(tplay);
	tplayr.top = sheight / 5 * 2.5;
	tplayr.bottom = tplayr.top + textheight(tplay);

	texitr.left = swidth / 2 - textwidth(texit) / 2;
	texitr.right = texitr.left + textwidth(texit);
	texitr.top = sheight / 5 * 3;
	texitr.bottom = texitr.top + textheight(texit);


	outtextxy(tplayr.left, tplayr.top, tplay);
	outtextxy(texitr.left, texitr.top, texit);

	EndBatchDraw();//减少频闪

	while (true)
	{
		ExMessage mess;
		getmessage(&mess, EM_MOUSE);
		if (mess.lbutton)
		{
			if (PointInRect(mess.x, mess.y, tplayr))
			{
				return;
			}
			else if (PointInRect(mess.x, mess.y, texitr))
			{
				exit(0);
			}

		}
	}

}

void Over(unsigned long long& kill)
{
	TCHAR* str = new TCHAR[128];
	_stprintf_s(str, 128, _T("击杀数：%llu"), kill);

	settextcolor(RED);
	outtextxy(swidth / 2 - textwidth(str) / 2, sheight / 5, str);

	//键盘事件 （按回车键键盘返回）
	LPCTSTR info = _T("按Enter返回");
	settextstyle(20, 0, _T("黑体"));
	outtextxy(swidth - textwidth(info), sheight - textheight(info), info);

	while (true)
	{
		ExMessage mess;
		getmessage(&mess, EM_KEY);
		if (mess.vkcode == 0x0D)
		{
			return;
		}
	}
}

//背景，敌机，我方，子弹

class BK
{
public:
	BK(IMAGE& img)
		:img(img), y(-sheight)
	{

	}
	void  Show()
	{
		if (y == 0) { y = -sheight; }
		y += 4;
		putimage(0, y, &img);
	}
private:
	IMAGE& img;//滚动
	int x, y;
};

class Hero
{
public:
	Hero(IMAGE& img)
		:img(img)
	{	//矩形边框
		rect.left = swidth / 2 - img.getwidth() / 2;
		rect.top = sheight - img.getheight();
		rect.right = rect.left + img.getwidth();
		rect.bottom = sheight;
	}
	void Show()//鼠标控制我方飞机
	{
		putimage(rect.left, rect.top, &img);
	}
	void Control()
	{
		ExMessage mess;
		if (peekmessage(&mess, EM_MOUSE))
		{
			rect.left = mess.x - img.getwidth() / 2;
			rect.top = mess.y - img.getheight() / 2;
			rect.right = rect.right = rect.left + img.getwidth();
			rect.bottom = rect.top + img.getheight();
		}
	}
	RECT& GetRect() { return rect; }
private:
	IMAGE& img;
	RECT rect;
};
class Enemy
{
public:
	Enemy(IMAGE& img, int x)
		:img(img)
	{
		rect.left = x;
		rect.right = rect.left + img.getwidth();
		rect.top = -img.getheight();
		rect.bottom = 0;

	}
	bool Show()//两种情况1.废除画面销毁 2，被击中
	{
		if (rect.top >= sheight)
		{
			return false;
		}
		rect.top += 4;
		rect.bottom += 4;
		putimage(rect.left, rect.top, &img);
		return true;
	}
	RECT& GetRect() { return rect; }
private:
	IMAGE& img;
	RECT rect;
};

class Bullet
{
public:
	Bullet(IMAGE& img, RECT pr)
		:img(img)
	{
		rect.left = pr.left + (pr.right - pr.left) / 2 - img.getwidth() / 2;
		rect.right = rect.left + img.getwidth();
		rect.top = pr.top - img.getheight();
		rect.bottom = rect.top + img.getheight();
	}
	bool Show()
	{
		if (rect.bottom <= 0)
		{
			return false;
		}
		rect.top -= 3;
		rect.bottom -= 3;
		putimage(rect.left, rect.top, &img);

		return true;
	}
	RECT& GetRect() { return rect; }

private:
	IMAGE& img;
	RECT rect;

};

bool AddEnemy(vector<Enemy*>& es, IMAGE& enemyimg)
{
	Enemy* e = new Enemy(enemyimg, abs(rand()) % (swidth - enemyimg.getwidth()));

	for (auto& i : es)
	{
		if (RectDuangRect(i->GetRect(), e->GetRect()))
		{
			delete e;
			return false;
		}
	}
	es.push_back(e);
	return true;
}

bool Play()
{
	setbkcolor(WHITE);//设置背景颜色刷新背景
	cleardevice();
	bool is_play = true;

	IMAGE heroimg, enemyimg, bkimg, bulletimg;
	loadimage(&heroimg, _T("C:\\Users\\34670\\Desktop\\images\\me1.png"));
	loadimage(&bkimg, _T("C:\\Users\\34670\\Desktop\\images\\background2.png"), swidth, sheight * 2);
	loadimage(&enemyimg, _T("C:\\Users\\34670\\Desktop\\images\\enemy2.png"));
	loadimage(&bulletimg, _T("C:\\Users\\34670\\Desktop\\images\\bullet1.png"));

	BK bk = BK(bkimg);
	Hero hp = Hero(heroimg);//实例化

	vector<Enemy*>es;//敌机的类
	vector<Bullet*>bs;
	int bsing = 0;

	unsigned long long kill = 0;

	for (int i = 0;i < 5;i++)
	{

		AddEnemy(es, enemyimg);
	}
	while (is_play)//游戏循环
	{
		bsing++;
		if (bsing == 10)
		{
			bsing = 0;
			bs.push_back(new Bullet(bulletimg, hp.GetRect()));
		}
		BeginBatchDraw();//批量绘图

		//解决延时问题
		bk.Show();
		Sleep(6);//
		flushmessage();//刷新
		Sleep(2);//等待
		hp.Control();
		if (_kbhit())
		{
			char v = _getch();
			if (v == 0x20)
			{
				Sleep(500);
				while (true)
				{
					if (_kbhit())
					{
						v = _getch();
						if (v == 0x20)//空格
						{
							break;
						}
					}
					Sleep(16);
				}
			}

		}
		hp.Show();
		for (auto& i : bs)
		{
			i->Show();
		}

		auto it = es.begin();
		while (it != es.end())
		{
			if (RectDuangRect((*it)->GetRect(), hp.GetRect()))
			{
				is_play = false;
			}
			auto bit = bs.begin();
			while (bit != bs.end())
			{
				if (RectDuangRect((*bit)->GetRect(), (*it)->GetRect()))
				{
					delete(*it);
					es.erase(it);
					it = es.begin();
					delete(*bit);//删除子弹，删除敌机
					bs.erase(bit);

					kill++;

					break;
				}
				bit++;
			}
			if (!(*it)->Show())
			{
				delete(*it);
				it = es.erase(it);//删除函数,指向对象下一个元素
				it = es.begin();
			}
			it++;
		}
		for (int i = 0;i < 5 - es.size();i++)
		{
			AddEnemy(es, enemyimg);
		}

		EndBatchDraw();//关闭批量绘图
	}
	Over(kill);

	return true;
}
int main()
{
	//easyx初始界面



	initgraph(swidth, sheight, EW_NOMINIMIZE | EW_SHOWCONSOLE);
	//IMAGE img;
	//loadimage(&img,"C:\\Users\\34670\\Desktop\\images\\me1.png");
	//putimage(0, 0, &img);
	//getmessage(EM_CHAR);

	bool is_live = true;
	while (is_live)//持续循环
	{
		Welcome();
		is_live = Play();//游戏结束
		//play函数开始游戏
	}
	return 0;
}