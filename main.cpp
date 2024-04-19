#include <Novice.h>
#include"Vector3.h"
#include"Matrix4x4.h"
#include<cmath>


const char kWindowTitle[] = "LD2B_04_コマツザキ_カガリ_タイトル";


// 表示の関数化
static const int kRowHeight = 20;
static const int kColumnWidth = 60;

void VectorScreenPrintf(int x, int y, const Vector3& vector, const char* label)
{
	Novice::ScreenPrintf(x, y, "%.02f", vector.x);
	Novice::ScreenPrintf(x + kColumnWidth, y, "%.02f", vector.y);
	Novice::ScreenPrintf(x + kColumnWidth * 2, y, "%.02f", vector.z);
	Novice::ScreenPrintf(x + kColumnWidth * 3, y, "%s", label);
}

// 関数のプロトタイプ宣言

// クロス積
Vector3 Cross(const Vector3& v1, const Vector3& v2);
// 平行移動行列
Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
// X軸回転行列
Matrix4x4 MakeRotateXMatrix(const Vector3& rotate);
// Y軸回転行列
Matrix4x4 MakeRotateYMatrix(Vector3 rotate);
// Z軸回転行列
Matrix4x4 MakeRotateZMatrix(Vector3 rotate);
// XYZ合成
Matrix4x4 Multiply(const Matrix4x4& rotateX, const Matrix4x4& rotateYZ);
// アフィン変換
Matrix4x4 MakeAffineMatrix(const Matrix4x4& S, const Matrix4x4& R, const Matrix4x4& T);



// 関数の定義

// クロス積
Vector3 Cross(const Vector3& v1, const Vector3& v2)
{
	Vector3 result{};

	result.x = (v1.y * v2.z) - (v1.z * v2.y);
	result.y = (v1.z * v2.x) - (v1.x * v2.z);
	result.z = (v1.x * v2.y) - (v1.y * v2.x);

	return result;
}

// 平行移動行列
Matrix4x4 MakeTranslateMatrix(const Vector3& translate)
{
	Matrix4x4 result{};

	for (int i = 0; i < 4; i++)
	{
		result.m[i][i] = 1;
	}
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;

	return result;
}

// X軸回転行列
Matrix4x4 MakeRotateXMatrix(const Vector3& rotate)
{
	Matrix4x4 result{};

	result.m[1][1] = std::cos(rotate.x);
	result.m[1][2] = std::sin(rotate.x);
	result.m[2][1] = -std::sin(rotate.x);
	result.m[2][2] = std::cos(rotate.x);
	result.m[0][0] = 1;
	result.m[3][3] = 1;

	return result;
}

// Y軸回転行列
Matrix4x4 MakeRotateYMatrix(Vector3 rotate)
{
	Matrix4x4 result{};

	result.m[0][0] = std::cos(rotate.y);
	result.m[0][2] = -std::sin(rotate.y);
	result.m[1][1] = 1;
	result.m[2][0] = std::sin(rotate.y);
	result.m[2][2] = std::cos(rotate.y);
	result.m[3][3] = 1;

	return result;
}

// Z軸回転行列
Matrix4x4 MakeRotateZMatrix(Vector3 rotate)
{
	Matrix4x4 result{};

	result.m[0][0] = std::cos(rotate.z);
	result.m[0][1] = std::sin(rotate.z);
	result.m[1][0] = -std::sin(rotate.z);
	result.m[1][1] = std::cos(rotate.z);
	result.m[2][2] = 1;
	result.m[3][3] = 1;

	return result;
}

// XYZ合成
Matrix4x4 Multiply(const Matrix4x4& rotateX, const Matrix4x4& rotateYZ)
{
	Matrix4x4 result{};

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				result.m[i][j] += rotateX.m[i][k] * rotateYZ.m[k][j];
			}
		}
	}

	return result;
}

// アフィン変換
Matrix4x4 MakeAffineMatrix(const Matrix4x4& S, const Matrix4x4& R, const Matrix4x4& T)
{
	Matrix4x4 SR{};
	Matrix4x4 result{};

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				SR.m[i][j] += S.m[i][k] * R.m[k][j];
			}
		}
	}
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				result.m[i][j] += SR.m[i][k] * T.m[k][j];
			}
		}
	}

	return result;
}




// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};


	Vector3 v1{ 1.2f,-3.9f,2.5f };
	Vector3 v2{ 2.8f,0.4f,-1.3f };

	Vector3 rotate{};
	Vector3 translate{};


	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		Vector3 cross = Cross(v1, v2);
		Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);
		Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate);
		Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate);
		Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate);
		Matrix4x4 rotateXYZMatrix = Multiply(rotateXMatrix, Multiply(rotateYMatrix, rotateZMatrix));
		Matrix4x4 worldMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, rotateXYZMatrix, translateMatrix);
		Matrix4x4 cameraMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, cameraPosition);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		VectorScreenPrintf(0, 0, cross, "Cross");

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}
