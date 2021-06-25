#include <curses.h>
#include <iostream>
#include <time.h>
#include <stdlib.h>
//ぷよの色を表すの列挙型
//NONEが無し，RED,BLUE,..が色を表す
enum puyocolor
{
	NONE,
	RED,
	BLUE,
	GREEN,
	YELLOW
};

class PuyoArray
{
private:
	//盤面状態
	puyocolor *data;
	//盤面の行数，列数
	unsigned int data_line;
	unsigned int data_column;

	//メモリ開放
	void Release()
	{
		if (data == NULL)
		{
			return;
		}

		delete[] data;
		data = NULL;
	}

public:
	//コンストラクタ
	PuyoArray() : data(NULL), data_line(0), data_column(0) {}
	//デストラクタ
	~PuyoArray() { Release(); }
	//盤面サイズ変更
	void ChangeSize(unsigned int line, unsigned int column)
	{
		Release();

		//新しいサイズでメモリ確保
		data = new puyocolor[line * column];

		data_line = line;
		data_column = column;
	}

	//盤面の行数を返す
	unsigned int GetLine()
	{
		return data_line;
	}

	//盤面の列数を返す
	unsigned int GetColumn()
	{
		return data_column;
	}

	//盤面の指定された位置の値を返す
	puyocolor GetValue(unsigned int y, unsigned int x)
	{
		if (y >= GetLine() || x >= GetColumn())
		{
			//引数の値が正しくない
			return NONE;
		}

		return data[y * GetColumn() + x];
	}

	//盤面の指定された位置に値を書き込む
	void SetValue(unsigned int y, unsigned int x, puyocolor value)
	{
		if (y >= GetLine() || x >= GetColumn())
		{
			//引数の値が正しくない
			return;
		}

		data[y * GetColumn() + x] = value;
	}
};

class PuyoArrayActive : public PuyoArray
{
private:
	//ぷよの回転状態を管理
	int puyorotate;

public:
	//コンストラクタ
	PuyoArrayActive() : puyorotate(0) {}

	//アクセッサ
	int GetRotate()
	{
		return puyorotate;
	}

	void SetRotate(int i)
	{
		puyorotate = i;
	}
};
class PuyoArrayStack : public PuyoArray
{
};

class PuyoControl
{
public:
	//盤面に新しいぷよ生成
	void GeneratePuyo(PuyoArrayActive &puyo)
	{
		srand(time(NULL));
		puyocolor newpuyo1;
		switch (rand() % 4)
		{
		case 0:
			newpuyo1 = RED;
			break;
		case 1:
			newpuyo1 = BLUE;
			break;
		case 2:
			newpuyo1 = GREEN;
			break;
		case 3:
			newpuyo1 = YELLOW;
			break;
		}

		srand(time(NULL) + 1);
		puyocolor newpuyo2;
		switch (rand() % 4)
		{
		case 0:
			newpuyo2 = RED;
			break;
		case 1:
			newpuyo2 = BLUE;
			break;
		case 2:
			newpuyo2 = GREEN;
			break;
		case 3:
			newpuyo2 = YELLOW;
			break;
		}

		puyo.SetValue(0, 5, newpuyo1);
		puyo.SetValue(0, 6, newpuyo2);

		//ぷよ回転状態を初期化
		puyo.SetRotate(0);
	}

	//ぷよの着地判定．着地判定があるとtrueを返す
	bool LandingPuyo(PuyoArrayActive &active, PuyoArrayStack &stack)
	{
		bool landed = false;

		for (int y = 0; y < active.GetLine(); y++)
		{
			for (int x = 0; x < active.GetColumn(); x++)
			{
				//ぷよが一番下または他のぷよの上に来たとき着地判定
				if (active.GetValue(y, x) != NONE && (y == active.GetLine() - 1 || stack.GetValue(y + 1, x) != NONE))
				{
					landed = true;
					//着地済みぷよにする
					stack.SetValue(y, x, active.GetValue(y, x));
					//同時に上下左右の落下中ぷよも着地済みにする
					//着地済みぷよがすでにあったらダメ
					if (stack.GetValue(y, x + 1) == NONE)
					{
						stack.SetValue(y, x + 1, active.GetValue(y, x + 1));
					}
					if (stack.GetValue(y, x - 1) == NONE)
					{
						stack.SetValue(y, x - 1, active.GetValue(y, x - 1));
					}
					if (stack.GetValue(y - 1, x) == NONE)
					{
						stack.SetValue(y - 1, x, active.GetValue(y - 1, x));
					}

					//着地判定されたぷよを消す．本処理は必要に応じて変更する．
					active.SetValue(y, x, NONE);
					active.SetValue(y, x + 1, NONE);
					active.SetValue(y, x - 1, NONE);
					active.SetValue(y - 1, x, NONE);
				}
			}
		}

		return landed;
	}

	//左に何もなければTrueを返す
	bool MoveLeftOk(PuyoArrayActive &active, PuyoArrayStack &stack)
	{
		for (int y = 0; y < active.GetLine(); y++)
		{
			for (int x = 0; x < active.GetColumn(); x++)
			{
				if (active.GetValue(y, x) != NONE)
				{
					if (stack.GetValue(y, x - 1) != NONE)
					{
						return false;
					}
				}
			}
		}
		return true;
	}

	//右に何もなければTrueを返す
	bool MoveRightOk(PuyoArrayActive &active, PuyoArrayStack &stack)
	{
		for (int y = 0; y < active.GetLine(); y++)
		{
			for (int x = 0; x < active.GetColumn(); x++)
			{
				if (active.GetValue(y, x) != NONE)
				{
					if (stack.GetValue(y, x + 1) != NONE)
					{
						return false;
					}
				}
			}
		}
		return true;
	}

	//左移動
	void MoveLeft(PuyoArrayActive &active)
	{
		//一時的格納場所メモリ確保
		puyocolor *puyo_temp = new puyocolor[active.GetLine() * active.GetColumn()];

		for (int i = 0; i < active.GetLine() * active.GetColumn(); i++)
		{
			puyo_temp[i] = NONE;
		}

		//1つ左の位置にpuyoactiveからpuyo_tempへとコピー
		for (int y = 0; y < active.GetLine(); y++)
		{
			for (int x = 0; x < active.GetColumn(); x++)
			{
				if (active.GetValue(y, x) == NONE)
				{
					continue;
				}

				if (0 < x && active.GetValue(y, x - 1) == NONE)
				{
					puyo_temp[y * active.GetColumn() + (x - 1)] = active.GetValue(y, x);
					//コピー後に元位置のpuyoactiveのデータは消す
					active.SetValue(y, x, NONE);
				}
				else
				{
					puyo_temp[y * active.GetColumn() + x] = active.GetValue(y, x);
				}
			}
		}

		//puyo_tempからpuyoactiveへコピー
		for (int y = 0; y < active.GetLine(); y++)
		{
			for (int x = 0; x < active.GetColumn(); x++)
			{
				active.SetValue(y, x, puyo_temp[y * active.GetColumn() + x]);
			}
		}

		//一時的格納場所メモリ解放
		delete[] puyo_temp;
	}

	//右移動
	void MoveRight(PuyoArrayActive &puyo)
	{
		//一時的格納場所メモリ確保
		puyocolor *puyo_temp = new puyocolor[puyo.GetLine() * puyo.GetColumn()];

		for (int i = 0; i < puyo.GetLine() * puyo.GetColumn(); i++)
		{
			puyo_temp[i] = NONE;
		}

		//1つ右の位置にpuyoactiveからpuyo_tempへとコピー
		for (int y = 0; y < puyo.GetLine(); y++)
		{
			for (int x = puyo.GetColumn() - 1; x >= 0; x--)
			{
				if (puyo.GetValue(y, x) == NONE)
				{
					continue;
				}

				if (x < puyo.GetColumn() - 1 && puyo.GetValue(y, x + 1) == NONE)
				{
					puyo_temp[y * puyo.GetColumn() + (x + 1)] = puyo.GetValue(y, x);
					//コピー後に元位置のpuyoactiveのデータは消す
					puyo.SetValue(y, x, NONE);
				}
				else
				{
					puyo_temp[y * puyo.GetColumn() + x] = puyo.GetValue(y, x);
				}
			}
		}

		//puyo_tempからpuyoactiveへコピー
		for (int y = 0; y < puyo.GetLine(); y++)
		{
			for (int x = 0; x < puyo.GetColumn(); x++)
			{
				puyo.SetValue(y, x, puyo_temp[y * puyo.GetColumn() + x]);
			}
		}

		//一時的格納場所メモリ解放
		delete[] puyo_temp;
	}

	//下移動
	void MoveDown(PuyoArrayActive &puyo)
	{
		//一時的格納場所メモリ確保
		puyocolor *puyo_temp = new puyocolor[puyo.GetLine() * puyo.GetColumn()];

		for (int i = 0; i < puyo.GetLine() * puyo.GetColumn(); i++)
		{
			puyo_temp[i] = NONE;
		}

		//1つ下の位置にpuyoactiveからpuyo_tempへとコピー
		for (int y = puyo.GetLine() - 1; y >= 0; y--)
		{
			for (int x = 0; x < puyo.GetColumn(); x++)
			{
				if (puyo.GetValue(y, x) == NONE)
				{
					continue;
				}

				if (y < puyo.GetLine() - 1 && puyo.GetValue(y + 1, x) == NONE)
				{
					puyo_temp[(y + 1) * puyo.GetColumn() + x] = puyo.GetValue(y, x);
					//コピー後に元位置のpuyoactiveのデータは消す
					puyo.SetValue(y, x, NONE);
				}
				else
				{
					puyo_temp[y * puyo.GetColumn() + x] = puyo.GetValue(y, x);
				}
			}
		}

		//puyo_tempからpuyoactiveへコピー
		for (int y = 0; y < puyo.GetLine(); y++)
		{
			for (int x = 0; x < puyo.GetColumn(); x++)
			{
				puyo.SetValue(y, x, puyo_temp[y * puyo.GetColumn() + x]);
			}
		}

		//一時的格納場所メモリ解放
		delete[] puyo_temp;
	}

	//ういているぷよを一つ下に落とす
	void FallPuyo(PuyoArrayStack &puyo)
	{
		//一時的格納場所メモリ確保
		puyocolor *puyo_temp = new puyocolor[puyo.GetLine() * puyo.GetColumn()];

		for (int i = 0; i < puyo.GetLine() * puyo.GetColumn(); i++)
		{
			puyo_temp[i] = NONE;
		}

		//1つ下の位置にpuyoactiveからpuyo_tempへとコピー
		for (int y = puyo.GetLine() - 1; y >= 0; y--)
		{
			for (int x = 0; x < puyo.GetColumn(); x++)
			{
				if (puyo.GetValue(y, x) == NONE)
				{
					continue;
				}

				if (y < puyo.GetLine() - 1 && puyo.GetValue(y + 1, x) == NONE)
				{
					puyo_temp[(y + 1) * puyo.GetColumn() + x] = puyo.GetValue(y, x);
					//コピー後に元位置のpuyoactiveのデータは消す
					puyo.SetValue(y, x, NONE);
				}
				else
				{
					puyo_temp[y * puyo.GetColumn() + x] = puyo.GetValue(y, x);
				}
			}
		}

		//puyo_tempからpuyoactiveへコピー
		for (int y = 0; y < puyo.GetLine(); y++)
		{
			for (int x = 0; x < puyo.GetColumn(); x++)
			{
				puyo.SetValue(y, x, puyo_temp[y * puyo.GetColumn() + x]);
			}
		}

		//一時的格納場所メモリ解放
		delete[] puyo_temp;
	}
	//ぷよ消滅処理を全座標で行う
	//消滅したぷよの数を返す
	int VanishPuyo(PuyoArrayStack &puyostack)
	{
		int vanishednumber = 0;
		for (int y = 0; y < puyostack.GetLine(); y++)
		{
			for (int x = 0; x < puyostack.GetColumn(); x++)
			{
				vanishednumber += VanishPuyo(puyostack, y, x);
			}
		}

		return vanishednumber;
	}

	//ぷよ消滅処理を座標(x,y)で行う
	//消滅したぷよの数を返す
	int VanishPuyo(PuyoArrayStack &puyostack, unsigned int y, unsigned int x)
	{
		//判定個所にぷよがなければ処理終了
		if (puyostack.GetValue(y, x) == NONE)
		{
			return 0;
		}

		//判定状態を表す列挙型
		//NOCHECK判定未実施，CHECKINGが判定対象，CHECKEDが判定済み
		enum checkstate
		{
			NOCHECK,
			CHECKING,
			CHECKED
		};

		//判定結果格納用の配列
		enum checkstate *field_array_check;
		field_array_check = new enum checkstate[puyostack.GetLine() * puyostack.GetColumn()];

		//配列初期化
		for (int i = 0; i < puyostack.GetLine() * puyostack.GetColumn(); i++)
		{
			field_array_check[i] = NOCHECK;
		}

		//座標(x,y)を判定対象にする
		field_array_check[y * puyostack.GetColumn() + x] = CHECKING;

		//判定対象が1つもなくなるまで，判定対象の上下左右に同じ色のぷよがあるか確認し，あれば新たな判定対象にする
		bool checkagain = true;
		while (checkagain)
		{
			checkagain = false;

			for (int y = 0; y < puyostack.GetLine(); y++)
			{
				for (int x = 0; x < puyostack.GetColumn(); x++)
				{
					//(x,y)に判定対象がある場合
					if (field_array_check[y * puyostack.GetColumn() + x] == CHECKING)
					{
						//(x+1,y)の判定
						if (x < puyostack.GetColumn() - 1)
						{
							//(x+1,y)と(x,y)のぷよの色が同じで，(x+1,y)のぷよが判定未実施か確認
							if (puyostack.GetValue(y, x + 1) == puyostack.GetValue(y, x) && field_array_check[y * puyostack.GetColumn() + (x + 1)] == NOCHECK)
							{
								//(x+1,y)を判定対象にする
								field_array_check[y * puyostack.GetColumn() + (x + 1)] = CHECKING;
								checkagain = true;
							}
						}

						//(x-1,y)の判定
						if (x > 0)
						{
							if (puyostack.GetValue(y, x - 1) == puyostack.GetValue(y, x) && field_array_check[y * puyostack.GetColumn() + (x - 1)] == NOCHECK)
							{
								field_array_check[y * puyostack.GetColumn() + (x - 1)] = CHECKING;
								checkagain = true;
							}
						}

						//(x,y+1)の判定
						if (y < puyostack.GetLine() - 1)
						{
							if (puyostack.GetValue(y + 1, x) == puyostack.GetValue(y, x) && field_array_check[(y + 1) * puyostack.GetColumn() + x] == NOCHECK)
							{
								field_array_check[(y + 1) * puyostack.GetColumn() + x] = CHECKING;
								checkagain = true;
							}
						}

						//(x,y-1)の判定
						if (y > 0)
						{
							if (puyostack.GetValue(y - 1, x) == puyostack.GetValue(y, x) && field_array_check[(y - 1) * puyostack.GetColumn() + x] == NOCHECK)
							{
								field_array_check[(y - 1) * puyostack.GetColumn() + x] = CHECKING;
								checkagain = true;
							}
						}

						//(x,y)を判定済みにする
						field_array_check[y * puyostack.GetColumn() + x] = CHECKED;
					}
				}
			}
		}

		//判定済みの数をカウント
		int puyocount = 0;
		for (int i = 0; i < puyostack.GetLine() * puyostack.GetColumn(); i++)
		{
			if (field_array_check[i] == CHECKED)
			{
				puyocount++;
			}
		}

		//4個以上あれば，判定済み座標のぷよを消す
		int vanishednumber = 0;
		if (4 <= puyocount)
		{
			for (int y = 0; y < puyostack.GetLine(); y++)
			{
				for (int x = 0; x < puyostack.GetColumn(); x++)
				{
					if (field_array_check[y * puyostack.GetColumn() + x] == CHECKED)
					{
						puyostack.SetValue(y, x, NONE);

						vanishednumber++;
					}
				}
			}
		}

		//メモリ解放
		delete[] field_array_check;

		return vanishednumber;
	}

	//回転
	//PuyoArrayActiveクラスのprivateメンバ変数として int puyorotate を宣言し，これに回転状態を記憶させている．
	//puyorotateにはコンストラクタ及びGeneratePuyo関数で値0を代入する必要あり．
	void Rotate(PuyoArrayActive &puyoactive, PuyoArrayStack &puyostack)
	{
		//フィールドをラスタ順に探索（最も上の行を左から右方向へチェックして，次に一つ下の行を左から右方向へチェックして，次にその下の行・・と繰り返す）し，先に発見される方をpuyo1, 次に発見される方をpuyo2に格納
		puyocolor puyo1, puyo2;
		int puyo1_x = 0;
		int puyo1_y = 0;
		int puyo2_x = 0;
		int puyo2_y = 0;

		bool findingpuyo1 = true;
		for (int y = 0; y < puyoactive.GetLine(); y++)
		{
			for (int x = 0; x < puyoactive.GetColumn(); x++)
			{
				if (puyoactive.GetValue(y, x) != NONE)
				{
					if (findingpuyo1)
					{
						puyo1 = puyoactive.GetValue(y, x);
						puyo1_x = x;
						puyo1_y = y;
						findingpuyo1 = false;
					}
					else
					{
						puyo2 = puyoactive.GetValue(y, x);
						puyo2_x = x;
						puyo2_y = y;
					}
				}
			}
		}

		//回転前のぷよを消す
		puyoactive.SetValue(puyo1_y, puyo1_x, NONE);
		puyoactive.SetValue(puyo2_y, puyo2_x, NONE);

		//操作中ぷよの回転
		switch (puyoactive.GetRotate())
		{
		case 0:
			//回転パターン
			//RB -> R
			//      B
			//Rがpuyo1, Bがpuyo2
			if (puyo2_x <= 0 || puyo2_y >= puyoactive.GetLine() - 1 || puyostack.GetValue(puyo1_y + 1, puyo1_x) != NONE) //もし回転した結果field_arrayの範囲外に出るなら回転しない
			{
				puyoactive.SetValue(puyo1_y, puyo1_x, puyo1);
				puyoactive.SetValue(puyo2_y, puyo2_x, puyo2);
				break;
			}

			//回転後の位置にぷよを置く
			puyoactive.SetValue(puyo1_y, puyo1_x, puyo1);
			puyoactive.SetValue(puyo2_y + 1, puyo2_x - 1, puyo2);
			//次の回転パターンの設定
			puyoactive.SetRotate(1);
			break;

		case 1:
			//回転パターン
			//R -> BR
			//B
			//Rがpuyo1, Bがpuyo2
			if (puyo2_x <= 0 || puyo2_y <= 0 || puyostack.GetValue(puyo1_y, puyo1_x - 1) != NONE) //もし回転した結果field_arrayの範囲外に出るなら回転しない
			{
				puyoactive.SetValue(puyo1_y, puyo1_x, puyo1);
				puyoactive.SetValue(puyo2_y, puyo2_x, puyo2);
				break;
			}

			//回転後の位置にぷよを置く
			puyoactive.SetValue(puyo1_y, puyo1_x, puyo1);
			puyoactive.SetValue(puyo2_y - 1, puyo2_x - 1, puyo2);

			//次の回転パターンの設定
			puyoactive.SetRotate(2);
			break;

		case 2:
			//回転パターン
			//      B
			//BR -> R
			//Bがpuyo1, Rがpuyo2
			if (puyo1_x >= puyoactive.GetColumn() - 1 || puyo1_y <= 0 || puyostack.GetValue(puyo2_y - 1, puyo2_x) != NONE) //もし回転した結果field_arrayの範囲外に出るなら回転しない
			{
				puyoactive.SetValue(puyo1_y, puyo1_x, puyo1);
				puyoactive.SetValue(puyo2_y, puyo2_x, puyo2);
				break;
			}

			//回転後の位置にぷよを置く
			puyoactive.SetValue(puyo1_y - 1, puyo1_x + 1, puyo1);
			puyoactive.SetValue(puyo2_y, puyo2_x, puyo2);

			//次の回転パターンの設定
			puyoactive.SetRotate(3);
			break;

		case 3:
			//回転パターン
			//B
			//R -> RB
			//Bがpuyo1, Rがpuyo2
			if (puyo1_x >= puyoactive.GetColumn() - 1 || puyo1_y >= puyoactive.GetLine() - 1 || puyostack.GetValue(puyo2_y, puyo2_x + 1) != NONE) //もし回転した結果field_arrayの範囲外に出るなら回転しない
			{
				puyoactive.SetValue(puyo1_y, puyo1_x, puyo1);
				puyoactive.SetValue(puyo2_y, puyo2_x, puyo2);
				break;
			}

			//回転後の位置にぷよを置く
			puyoactive.SetValue(puyo1_y + 1, puyo1_x + 1, puyo1);
			puyoactive.SetValue(puyo2_y, puyo2_x, puyo2);

			//次の回転パターンの設定
			puyoactive.SetRotate(0);
			break;

		default:
			break;
		}
	}

	//回転
	//PuyoArrayActiveクラスのprivateメンバ変数として int puyorotate を宣言し，これに回転状態を記憶させている．
	//puyorotateにはコンストラクタ及びGeneratePuyo関数で値0を代入する必要あり．
	void RotateL(PuyoArrayActive &puyoactive, PuyoArrayStack &puyostack)
	{
		//フィールドをラスタ順に探索（最も上の行を左から右方向へチェックして，次に一つ下の行を左から右方向へチェックして，次にその下の行・・と繰り返す）し，先に発見される方をpuyo1, 次に発見される方をpuyo2に格納
		puyocolor puyo1, puyo2;
		int puyo1_x = 0;
		int puyo1_y = 0;
		int puyo2_x = 0;
		int puyo2_y = 0;

		bool findingpuyo1 = true;
		for (int y = 0; y < puyoactive.GetLine(); y++)
		{
			for (int x = 0; x < puyoactive.GetColumn(); x++)
			{
				if (puyoactive.GetValue(y, x) != NONE)
				{
					if (findingpuyo1)
					{
						puyo1 = puyoactive.GetValue(y, x);
						puyo1_x = x;
						puyo1_y = y;
						findingpuyo1 = false;
					}
					else
					{
						puyo2 = puyoactive.GetValue(y, x);
						puyo2_x = x;
						puyo2_y = y;
					}
				}
			}
		}

		//回転前のぷよを消す
		puyoactive.SetValue(puyo1_y, puyo1_x, NONE);
		puyoactive.SetValue(puyo2_y, puyo2_x, NONE);

		//操作中ぷよの回転
		switch (puyoactive.GetRotate())
		{
		case 0:
			//回転パターン
			//RB -> B
			//      R
			//Rがpuyo1, Bがpuyo2
			if (puyo2_x <= 0 || puyo2_y <= 0 || puyostack.GetValue(puyo1_y - 1, puyo1_x) != NONE) //もし回転した結果field_arrayの範囲外に出るなら回転しない
			{
				puyoactive.SetValue(puyo1_y, puyo1_x, puyo1);
				puyoactive.SetValue(puyo2_y, puyo2_x, puyo2);
				break;
			}

			//回転後の位置にぷよを置く
			puyoactive.SetValue(puyo1_y, puyo1_x, puyo1);
			puyoactive.SetValue(puyo2_y - 1, puyo2_x - 1, puyo2);
			//次の回転パターンの設定
			puyoactive.SetRotate(3);
			break;

		case 1:
			//回転パターン
			//R -> RB
			//B
			//Rがpuyo1, Bがpuyo2
			if (puyo2_x >= puyoactive.GetColumn() - 1 || puyo2_y >= puyoactive.GetLine() - 1 || puyostack.GetValue(puyo1_y, puyo1_x + 1) != NONE) //もし回転した結果field_arrayの範囲外に出るなら回転しない
			{
				puyoactive.SetValue(puyo1_y, puyo1_x, puyo1);
				puyoactive.SetValue(puyo2_y, puyo2_x, puyo2);
				break;
			}

			//回転後の位置にぷよを置く
			puyoactive.SetValue(puyo1_y, puyo1_x, puyo1);
			puyoactive.SetValue(puyo2_y - 1, puyo2_x + 1, puyo2);

			//次の回転パターンの設定
			puyoactive.SetRotate(0);
			break;

		case 2:
			//回転パターン
			//      R
			//BR -> B
			//Bがpuyo1, Rがpuyo2
			if (puyo1_x < 0 || puyo1_y >= puyoactive.GetLine() - 1 || puyostack.GetValue(puyo1_y + 1, puyo1_x + 1) != NONE) //もし回転した結果field_arrayの範囲外に出るなら回転しない
			{
				puyoactive.SetValue(puyo1_y, puyo1_x, puyo1);
				puyoactive.SetValue(puyo2_y, puyo2_x, puyo2);
				break;
			}

			//回転後の位置にぷよを置く
			puyoactive.SetValue(puyo1_y + 1, puyo1_x + 1, puyo1);
			puyoactive.SetValue(puyo2_y, puyo2_x, puyo2);

			//次の回転パターンの設定
			puyoactive.SetRotate(1);
			break;

		case 3:
			//回転パターン
			//B
			//R -> BR
			//Bがpuyo1, Rがpuyo2
			if (puyo1_x <= 0 || puyo1_y <= 0 || puyostack.GetValue(puyo2_y, puyo2_x - 1) != NONE) //もし回転した結果field_arrayの範囲外に出るなら回転しない
			{
				puyoactive.SetValue(puyo1_y, puyo1_x, puyo1);
				puyoactive.SetValue(puyo2_y, puyo2_x, puyo2);
				break;
			}

			//回転後の位置にぷよを置く
			puyoactive.SetValue(puyo1_y + 1, puyo1_x - 1, puyo1);
			puyoactive.SetValue(puyo2_y, puyo2_x, puyo2);

			//次の回転パターンの設定
			puyoactive.SetRotate(2);
			break;

		default:
			break;
		}
	}
	bool FlyPuyo(PuyoArrayStack &puyostack)
	{
		for (int y = 0; y < puyostack.GetLine() - 1; y++) //一番下は探さない
		{
			for (int x = 0; x < puyostack.GetColumn(); x++)
			{
				if (puyostack.GetValue(y, x) != NONE && puyostack.GetValue(y + 1, x) == NONE) //宙に浮いているぷよをさがす
				{
					return true;
				}
			}
		}
		return false;
	}
};

//表示　落下中のぷよおよび着地済みのぷよを描画する
void Display(PuyoArrayActive &active, PuyoArrayStack &stack, int score)
{
	init_pair(0, COLOR_WHITE, COLOR_BLACK);
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_BLUE, COLOR_BLACK);
	init_pair(3, COLOR_GREEN, COLOR_BLACK);
	init_pair(4, COLOR_YELLOW, COLOR_BLACK);
	//落下中ぷよ表示
	for (int y = 0; y < active.GetLine(); y++)
	{
		for (int x = 0; x < active.GetColumn(); x++)
		{
			switch (active.GetValue(y, x))
			{
			case NONE:
				attrset(COLOR_PAIR(0));
				mvaddch(y, x, '.');
				break;
			case RED:
				attrset(COLOR_PAIR(1));
				mvaddch(y, x, 'R');
				break;
			case BLUE:
				attrset(COLOR_PAIR(2));
				mvaddch(y, x, 'B');
				break;
			case GREEN:
				attrset(COLOR_PAIR(3));
				mvaddch(y, x, 'G');
				break;
			case YELLOW:
				attrset(COLOR_PAIR(4));
				mvaddch(y, x, 'Y');
				break;
			default:
				mvaddch(y, x, '?');
				break;
			}
		}
	}

	//着地済みぷよ表示
	for (int y = 0; y < stack.GetLine(); y++)
	{
		for (int x = 0; x < stack.GetColumn(); x++)
		{
			switch (stack.GetValue(y, x))
			{
			case NONE:
				break; //上書き防止　何もしない
			case RED:
				attrset(COLOR_PAIR(1));
				mvaddch(y, x, 'R');
				break;
			case BLUE:
				attrset(COLOR_PAIR(2));
				mvaddch(y, x, 'B');
				break;
			case GREEN:
				attrset(COLOR_PAIR(3));
				mvaddch(y, x, 'G');
				break;
			case YELLOW:
				attrset(COLOR_PAIR(4));
				mvaddch(y, x, 'Y');
				break;
			default:
				mvaddch(y, x, '?');
				break;
			}
		}
	}

	//情報表示
	int count = 0;
	attrset(COLOR_PAIR(0));
	for (int y = 0; y < active.GetLine(); y++)
	{
		for (int x = 0; x < active.GetColumn(); x++)
		{
			if (active.GetValue(y, x) != NONE)
			{
				count++;
			}
		}
	}

	char msg[256];
	sprintf(msg, "SCORE:%d", score);
	mvaddstr(2, COLS - 35, msg);

	refresh();
}

//ここから実行される
int main(int argc, char **argv)
{
	//画面の初期化
	initscr();
	//カラー属性を扱うための初期化
	start_color();

	//キーを押しても画面に表示しない
	noecho();
	//キー入力を即座に受け付ける
	cbreak();

	curs_set(0);
	//キー入力受付方法指定
	keypad(stdscr, TRUE);

	//キー入力非ブロッキングモード
	timeout(0);

	//インスタンスの作成
	PuyoArrayActive active;
	PuyoArrayStack stack;
	PuyoControl control;

	//初期化処理
	active.ChangeSize(LINES / 2, COLS / 2);
	stack.ChangeSize(LINES / 2, COLS / 2); //フィールドは画面サイズの縦横1/2にする
	control.GeneratePuyo(active);					 //最初のぷよ生成

	int delay = 0;
	int waitCount = 20000;

	int puyostate = 0;

	bool flag = false; //trueなら連鎖終了

	int score = 0;
	//メイン処理ループ
	while (1)
	{
		//キー入力受付
		int ch;
		ch = getch();

		//Qの入力で終了
		if (ch == 'Q')
		{
			break;
		}

		//入力キーごとの処理
		switch (ch)
		{
		case KEY_LEFT:
			if (control.MoveLeftOk(active, stack))
				control.MoveLeft(active);
			break;
		case KEY_RIGHT:
			if (control.MoveRightOk(active, stack))
				control.MoveRight(active);
			break;
			//case KEY_DOWN:
			//control.MoveDown(active);
			//break;
		case 'z':
			//ぷよ回転処理
			control.Rotate(active, stack);
			break;
		case 'x':
			control.RotateL(active, stack);
			break;
		default:
			break;
		}

		//表示
		Display(active, stack, score);

		//処理速度調整のためのif文
		if (delay % waitCount == 0)
		{
			//ぷよ下に移動
			control.MoveDown(active);

			if (flag) //着地判定後、連鎖開始
			{
				while (control.FlyPuyo(stack))
				{
					control.FallPuyo(stack); //宙に浮いているぷよをできるだけ下へ
				}
				//ぷよが消えなければ連鎖終了
				if (control.VanishPuyo(stack) == 0)
				{
					control.GeneratePuyo(active);
					flag = false;
				}
				else
				{
					score += 1;
				}
			}

			if (control.LandingPuyo(active, stack))
			{
				flag = true; //着地確認→連鎖へ
			}
		}
		delay++;

		//表示
		//		Display(active, stack);
	}

	//画面をリセット
	endwin();

	return 0;
}
