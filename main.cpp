#include <Novice.h>
#include"Vector3.h"
#include"Matrix4x4.h"
#include<cmath>
#include"assert.h"


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
Matrix4x4 MakeAffineMatrix(const Vector3& S, const Vector3& R, const Vector3& T);
// 逆行列
Matrix4x4 Inverse(Matrix4x4 cameraMatrix);
// 透視投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
// ビューポート行列
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);
// 座標変換
Vector3 Transform(const Vector3& point, const Matrix4x4& transformMatrix);



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
Matrix4x4 MakeRotateXMatrix(float rotate)
{
	Matrix4x4 result{};

	result.m[1][1] = std::cos(rotate);
	result.m[1][2] = std::sin(rotate);
	result.m[2][1] = -std::sin(rotate);
	result.m[2][2] = std::cos(rotate);
	result.m[0][0] = 1;
	result.m[3][3] = 1;

	return result;
}

// Y軸回転行列
Matrix4x4 MakeRotateYMatrix(float rotate)
{
	Matrix4x4 result{};

	result.m[0][0] = std::cos(rotate);
	result.m[0][2] = -std::sin(rotate);
	result.m[1][1] = 1;
	result.m[2][0] = std::sin(rotate);
	result.m[2][2] = std::cos(rotate);
	result.m[3][3] = 1;

	return result;
}

// Z軸回転行列
Matrix4x4 MakeRotateZMatrix(float rotate)
{
	Matrix4x4 result{};

	result.m[0][0] = std::cos(rotate);
	result.m[0][1] = std::sin(rotate);
	result.m[1][0] = -std::sin(rotate);
	result.m[1][1] = std::cos(rotate);
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
Matrix4x4 MakeAffineMatrix(const Vector3& S, const Vector3& R, const Vector3& T)
{
	Matrix4x4 result{};

	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(R.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(R.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(R.z);
	Matrix4x4 rotateXYZMatrix = Multiply(rotateXMatrix, Multiply(rotateYMatrix, rotateZMatrix));


	result.m[0][0] = S.x * rotateXYZMatrix.m[0][0];
	result.m[0][1] = S.x * rotateXYZMatrix.m[0][1];
	result.m[0][2] = S.x * rotateXYZMatrix.m[0][2];
	result.m[1][0] = S.y * rotateXYZMatrix.m[1][0];
	result.m[1][1] = S.y * rotateXYZMatrix.m[1][1];
	result.m[1][2] = S.y * rotateXYZMatrix.m[1][2];
	result.m[2][0] = S.z * rotateXYZMatrix.m[2][0];
	result.m[2][1] = S.z * rotateXYZMatrix.m[2][1];
	result.m[2][2] = S.z * rotateXYZMatrix.m[2][2];
	result.m[3][0] = T.x;
	result.m[3][1] = T.y;
	result.m[3][2] = T.z;
	result.m[3][3] = 1;

	return result;
}


// 逆行列
Matrix4x4 Inverse(Matrix4x4 cameraMatrix)
{
	Matrix4x4 result{};

	float abs;//絶対値はint型にする

	// |A|
	abs = (cameraMatrix.m[0][0] * cameraMatrix.m[1][1] * cameraMatrix.m[2][2] * cameraMatrix.m[3][3]) + (cameraMatrix.m[0][0] * cameraMatrix.m[1][2] * cameraMatrix.m[2][3] * cameraMatrix.m[3][1]) + (cameraMatrix.m[0][0] * cameraMatrix.m[1][3] * cameraMatrix.m[2][1] * cameraMatrix.m[3][2])
		- (cameraMatrix.m[0][0] * cameraMatrix.m[1][3] * cameraMatrix.m[2][2] * cameraMatrix.m[3][1]) - (cameraMatrix.m[0][0] * cameraMatrix.m[1][2] * cameraMatrix.m[2][1] * cameraMatrix.m[3][3]) - (cameraMatrix.m[0][0] * cameraMatrix.m[1][1] * cameraMatrix.m[2][3] * cameraMatrix.m[3][2])
		- (cameraMatrix.m[0][1] * cameraMatrix.m[1][0] * cameraMatrix.m[2][2] * cameraMatrix.m[3][3]) - (cameraMatrix.m[0][2] * cameraMatrix.m[1][0] * cameraMatrix.m[2][3] * cameraMatrix.m[3][1]) - (cameraMatrix.m[0][3] * cameraMatrix.m[1][0] * cameraMatrix.m[2][1] * cameraMatrix.m[3][2])
		+ (cameraMatrix.m[0][3] * cameraMatrix.m[1][0] * cameraMatrix.m[2][2] * cameraMatrix.m[3][1]) + (cameraMatrix.m[0][2] * cameraMatrix.m[1][0] * cameraMatrix.m[2][1] * cameraMatrix.m[3][3]) + (cameraMatrix.m[0][1] * cameraMatrix.m[1][0] * cameraMatrix.m[2][3] * cameraMatrix.m[3][2])
		+ (cameraMatrix.m[0][1] * cameraMatrix.m[1][2] * cameraMatrix.m[2][0] * cameraMatrix.m[3][3]) + (cameraMatrix.m[0][2] * cameraMatrix.m[1][3] * cameraMatrix.m[2][0] * cameraMatrix.m[3][1]) + (cameraMatrix.m[0][3] * cameraMatrix.m[1][1] * cameraMatrix.m[2][0] * cameraMatrix.m[3][2])
		- (cameraMatrix.m[0][3] * cameraMatrix.m[1][2] * cameraMatrix.m[2][0] * cameraMatrix.m[3][1]) - (cameraMatrix.m[0][2] * cameraMatrix.m[1][1] * cameraMatrix.m[2][0] * cameraMatrix.m[3][3]) - (cameraMatrix.m[0][1] * cameraMatrix.m[1][3] * cameraMatrix.m[2][0] * cameraMatrix.m[3][2])
		- (cameraMatrix.m[0][1] * cameraMatrix.m[1][2] * cameraMatrix.m[2][3] * cameraMatrix.m[3][0]) - (cameraMatrix.m[0][2] * cameraMatrix.m[1][3] * cameraMatrix.m[2][1] * cameraMatrix.m[3][0]) - (cameraMatrix.m[0][3] * cameraMatrix.m[1][1] * cameraMatrix.m[2][2] * cameraMatrix.m[3][0])
		+ (cameraMatrix.m[0][3] * cameraMatrix.m[1][2] * cameraMatrix.m[2][1] * cameraMatrix.m[3][0]) + (cameraMatrix.m[0][2] * cameraMatrix.m[1][1] * cameraMatrix.m[2][3] * cameraMatrix.m[3][0]) + (cameraMatrix.m[0][1] * cameraMatrix.m[1][3] * cameraMatrix.m[2][2] * cameraMatrix.m[3][0]
			);

	// 1/A
	result.m[0][0] = 1.0f / abs * (
		(cameraMatrix.m[1][1] * cameraMatrix.m[2][2] * cameraMatrix.m[3][3]) + (cameraMatrix.m[1][2] * cameraMatrix.m[2][3] * cameraMatrix.m[3][1]) + (cameraMatrix.m[1][3] * cameraMatrix.m[2][1] * cameraMatrix.m[3][2])
		- (cameraMatrix.m[1][3] * cameraMatrix.m[2][2] * cameraMatrix.m[3][1]) - (cameraMatrix.m[1][2] * cameraMatrix.m[2][1] * cameraMatrix.m[3][3]) - (cameraMatrix.m[1][1] * cameraMatrix.m[2][3] * cameraMatrix.m[3][2])
		);
	result.m[0][1] = 1.0f / abs * (
		-(cameraMatrix.m[0][1] * cameraMatrix.m[2][2] * cameraMatrix.m[3][3]) - (cameraMatrix.m[0][2] * cameraMatrix.m[2][3] * cameraMatrix.m[3][1]) - (cameraMatrix.m[0][3] * cameraMatrix.m[2][1] * cameraMatrix.m[3][2])
		+ cameraMatrix.m[0][3] * cameraMatrix.m[2][2] * cameraMatrix.m[3][1] + cameraMatrix.m[0][2] * cameraMatrix.m[2][1] * cameraMatrix.m[3][3] + cameraMatrix.m[0][1] * cameraMatrix.m[2][3] * cameraMatrix.m[3][2]
		);
	result.m[0][2] = 1.0f / abs * (
		(cameraMatrix.m[0][1] * cameraMatrix.m[1][2] * cameraMatrix.m[3][3]) + (cameraMatrix.m[0][2] * cameraMatrix.m[1][3] * cameraMatrix.m[3][1]) + (cameraMatrix.m[0][3] * cameraMatrix.m[1][1] * cameraMatrix.m[3][2])
		- (cameraMatrix.m[0][3] * cameraMatrix.m[1][2] * cameraMatrix.m[3][1]) - (cameraMatrix.m[0][2] * cameraMatrix.m[1][1] * cameraMatrix.m[3][3]) - (cameraMatrix.m[0][1] * cameraMatrix.m[1][3] * cameraMatrix.m[3][2])
		);
	result.m[0][3] = 1.0f / abs * (
		-(cameraMatrix.m[0][1] * cameraMatrix.m[1][2] * cameraMatrix.m[2][3]) - (cameraMatrix.m[0][2] * cameraMatrix.m[1][3] * cameraMatrix.m[2][1]) - (cameraMatrix.m[0][3] * cameraMatrix.m[1][1] * cameraMatrix.m[2][2])
		+ (cameraMatrix.m[0][3] * cameraMatrix.m[1][2] * cameraMatrix.m[2][1]) + (cameraMatrix.m[0][2] * cameraMatrix.m[1][1] * cameraMatrix.m[2][3]) + (cameraMatrix.m[0][1] * cameraMatrix.m[1][3] * cameraMatrix.m[2][2])
		);

	result.m[1][0] = 1.0f / abs * (
		-(cameraMatrix.m[1][0] * cameraMatrix.m[2][2] * cameraMatrix.m[3][3]) - (cameraMatrix.m[1][2] * cameraMatrix.m[2][3] * cameraMatrix.m[3][0]) - (cameraMatrix.m[1][3] * cameraMatrix.m[2][0] * cameraMatrix.m[3][2])
		+ (cameraMatrix.m[1][3] * cameraMatrix.m[2][2] * cameraMatrix.m[3][0]) + (cameraMatrix.m[1][2] * cameraMatrix.m[2][0] * cameraMatrix.m[3][3]) + (cameraMatrix.m[1][0] * cameraMatrix.m[2][3] * cameraMatrix.m[3][2])
		);
	result.m[1][1] = 1.0f / abs * (
		(cameraMatrix.m[0][0] * cameraMatrix.m[2][2] * cameraMatrix.m[3][3]) + (cameraMatrix.m[0][2] * cameraMatrix.m[2][3] * cameraMatrix.m[3][0]) + (cameraMatrix.m[0][3] * cameraMatrix.m[2][0] * cameraMatrix.m[3][2])
		- (cameraMatrix.m[0][3] * cameraMatrix.m[2][2] * cameraMatrix.m[3][0]) - (cameraMatrix.m[0][2] * cameraMatrix.m[2][0] * cameraMatrix.m[3][3]) - (cameraMatrix.m[0][0] * cameraMatrix.m[2][3] * cameraMatrix.m[3][2])
		);
	result.m[1][2] = 1.0f / abs * (
		-(cameraMatrix.m[0][0] * cameraMatrix.m[1][2] * cameraMatrix.m[3][3]) - (cameraMatrix.m[0][2] * cameraMatrix.m[1][3] * cameraMatrix.m[3][0]) - (cameraMatrix.m[0][3] * cameraMatrix.m[1][0] * cameraMatrix.m[3][2])
		+ (cameraMatrix.m[0][3] * cameraMatrix.m[1][2] * cameraMatrix.m[3][0]) + (cameraMatrix.m[0][2] * cameraMatrix.m[1][0] * cameraMatrix.m[3][3]) + (cameraMatrix.m[0][0] * cameraMatrix.m[1][3] * cameraMatrix.m[3][2])
		);
	result.m[1][3] = 1.0f / abs * (
		(cameraMatrix.m[0][0] * cameraMatrix.m[1][2] * cameraMatrix.m[2][3]) + (cameraMatrix.m[0][2] * cameraMatrix.m[1][3] * cameraMatrix.m[2][0]) + (cameraMatrix.m[0][3] * cameraMatrix.m[1][0] * cameraMatrix.m[2][2])
		- (cameraMatrix.m[0][3] * cameraMatrix.m[1][2] * cameraMatrix.m[2][0]) - (cameraMatrix.m[0][2] * cameraMatrix.m[1][0] * cameraMatrix.m[2][3]) - (cameraMatrix.m[0][0] * cameraMatrix.m[1][3] * cameraMatrix.m[2][2])
		);

	result.m[2][0] = 1.0f / abs * (
		(cameraMatrix.m[1][0] * cameraMatrix.m[2][1] * cameraMatrix.m[3][3]) + (cameraMatrix.m[1][1] * cameraMatrix.m[2][3] * cameraMatrix.m[3][0]) + (cameraMatrix.m[1][3] * cameraMatrix.m[2][0] * cameraMatrix.m[3][1])
		- (cameraMatrix.m[1][3] * cameraMatrix.m[2][1] * cameraMatrix.m[3][0]) - (cameraMatrix.m[1][1] * cameraMatrix.m[2][0] * cameraMatrix.m[3][3]) - (cameraMatrix.m[1][0] * cameraMatrix.m[2][3] * cameraMatrix.m[3][1])
		);
	result.m[2][1] = 1.0f / abs * (
		-(cameraMatrix.m[0][0] * cameraMatrix.m[2][1] * cameraMatrix.m[3][3]) - (cameraMatrix.m[0][1] * cameraMatrix.m[2][3] * cameraMatrix.m[3][0]) - (cameraMatrix.m[0][3] * cameraMatrix.m[2][0] * cameraMatrix.m[3][1])
		+ (cameraMatrix.m[0][3] * cameraMatrix.m[2][1] * cameraMatrix.m[3][0]) + (cameraMatrix.m[0][1] * cameraMatrix.m[2][0] * cameraMatrix.m[3][3]) + (cameraMatrix.m[0][0] * cameraMatrix.m[2][3] * cameraMatrix.m[3][1])
		);
	result.m[2][2] = 1.0f / abs * (
		(cameraMatrix.m[0][0] * cameraMatrix.m[1][1] * cameraMatrix.m[3][3]) + (cameraMatrix.m[0][1] * cameraMatrix.m[1][3] * cameraMatrix.m[3][0]) + (cameraMatrix.m[0][3] * cameraMatrix.m[1][0] * cameraMatrix.m[3][1])
		- (cameraMatrix.m[0][3] * cameraMatrix.m[1][1] * cameraMatrix.m[3][0]) - (cameraMatrix.m[0][1] * cameraMatrix.m[1][0] * cameraMatrix.m[3][3]) - (cameraMatrix.m[0][0] * cameraMatrix.m[1][3] * cameraMatrix.m[3][1])
		);
	result.m[2][3] = 1.0f / abs * (
		-(cameraMatrix.m[0][0] * cameraMatrix.m[1][1] * cameraMatrix.m[2][3]) - (cameraMatrix.m[0][1] * cameraMatrix.m[1][3] * cameraMatrix.m[2][0]) - (cameraMatrix.m[0][3] * cameraMatrix.m[1][0] * cameraMatrix.m[2][1])
		+ (cameraMatrix.m[0][3] * cameraMatrix.m[1][1] * cameraMatrix.m[2][0]) + (cameraMatrix.m[0][1] * cameraMatrix.m[1][0] * cameraMatrix.m[2][3]) + (cameraMatrix.m[0][0] * cameraMatrix.m[1][3] * cameraMatrix.m[2][1])
		);

	result.m[3][0] = 1.0f / abs * (
		-(cameraMatrix.m[1][0] * cameraMatrix.m[2][1] * cameraMatrix.m[3][2]) - (cameraMatrix.m[1][1] * cameraMatrix.m[2][2] * cameraMatrix.m[3][0]) - (cameraMatrix.m[1][2] * cameraMatrix.m[2][0] * cameraMatrix.m[3][1])
		+ (cameraMatrix.m[1][2] * cameraMatrix.m[2][1] * cameraMatrix.m[3][0]) + (cameraMatrix.m[1][1] * cameraMatrix.m[2][0] * cameraMatrix.m[3][2]) + (cameraMatrix.m[1][0] * cameraMatrix.m[2][2] * cameraMatrix.m[3][1])
		);
	result.m[3][1] = 1.0f / abs * (
		(cameraMatrix.m[0][0] * cameraMatrix.m[2][1] * cameraMatrix.m[3][2]) + (cameraMatrix.m[0][1] * cameraMatrix.m[2][2] * cameraMatrix.m[3][0]) + (cameraMatrix.m[0][2] * cameraMatrix.m[2][0] * cameraMatrix.m[3][1])
		- (cameraMatrix.m[0][2] * cameraMatrix.m[2][1] * cameraMatrix.m[3][0]) - (cameraMatrix.m[0][1] * cameraMatrix.m[2][0] * cameraMatrix.m[3][2]) - (cameraMatrix.m[0][0] * cameraMatrix.m[2][2] * cameraMatrix.m[3][1])
		);
	result.m[3][2] = 1.0f / abs * (
		-(cameraMatrix.m[0][0] * cameraMatrix.m[1][1] * cameraMatrix.m[3][2]) - (cameraMatrix.m[0][1] * cameraMatrix.m[1][2] * cameraMatrix.m[3][0]) - (cameraMatrix.m[0][2] * cameraMatrix.m[1][0] * cameraMatrix.m[3][1])
		+ (cameraMatrix.m[0][2] * cameraMatrix.m[1][1] * cameraMatrix.m[3][0]) + (cameraMatrix.m[0][1] * cameraMatrix.m[1][0] * cameraMatrix.m[3][2]) + (cameraMatrix.m[0][0] * cameraMatrix.m[1][2] * cameraMatrix.m[3][1])
		);
	result.m[3][3] = 1.0f / abs * (
		(cameraMatrix.m[0][0] * cameraMatrix.m[1][1] * cameraMatrix.m[2][2]) + (cameraMatrix.m[0][1] * cameraMatrix.m[1][2] * cameraMatrix.m[2][0]) + (cameraMatrix.m[0][2] * cameraMatrix.m[1][0] * cameraMatrix.m[2][1])
		- (cameraMatrix.m[0][2] * cameraMatrix.m[1][1] * cameraMatrix.m[2][0]) - (cameraMatrix.m[0][1] * cameraMatrix.m[1][0] * cameraMatrix.m[2][2]) - (cameraMatrix.m[0][0] * cameraMatrix.m[1][2] * cameraMatrix.m[2][1])
		);

	
	return result;
}


// 透視投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip)
{
	Matrix4x4 result{};

	result.m[0][0] = (1 / aspectRatio) * 1 / std::tan(fovY / 2);
	result.m[1][1] = 1 / std::tan(fovY / 2);
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1;
	result.m[3][2] = (-nearClip * farClip) / (farClip - nearClip);

	return result;
}


// ビューポート変換行列
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth)
{
	Matrix4x4 result{};

	result.m[0][0] = width / 2;
	result.m[1][1] = -height / 2;
	result.m[2][2] = maxDepth - minDepth;
	result.m[3][0] = left + (width / 2);
	result.m[3][1] = top + (height / 2);
	result.m[3][2] = minDepth;
	result.m[3][3] = 1;

	return result;
}


// 座標変換
Vector3 Transform(const Vector3& point, const Matrix4x4& transformMatrix)
{
	Vector3 result{};

	result.x = point.x * transformMatrix.m[0][0] + point.y * transformMatrix.m[1][0] + point.z * transformMatrix.m[2][0] + 1.0f * transformMatrix.m[3][0];
	result.y = point.x * transformMatrix.m[0][1] + point.y * transformMatrix.m[1][1] + point.z * transformMatrix.m[2][1] + 1.0f * transformMatrix.m[3][1];
	result.z = point.x * transformMatrix.m[0][2] + point.y * transformMatrix.m[1][2] + point.z * transformMatrix.m[2][2] + 1.0f * transformMatrix.m[3][2];
	float w = point.x * transformMatrix.m[0][3] + point.y * transformMatrix.m[1][3] + point.z * transformMatrix.m[2][3] + 1.0f * transformMatrix.m[3][3];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	result.z /= w;

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

	Vector3 cameraPosition{ 0.0f,0.0f,-300.0f };

	Vector3 kLocalVertices[3] = {};
	kLocalVertices[0] = {0.0f,20.0f,0.0f};
	kLocalVertices[1] = {60.0f,-20.0f,0.0f};
	kLocalVertices[2] = {-60.0f,-20.0f,0.0f};

	

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


		// 回転
		rotate.y += 0.01f;

		// 移動
		if (keys[DIK_W])
		{
			translate.z+=5.0f;
		}
		if (keys[DIK_S])
		{
			translate.z -= 5.0f;
		}
		if (keys[DIK_A])
		{
			translate.x -= 5.0f;
		}
		if (keys[DIK_D])
		{
			translate.x += 5.0f;
		}

		Vector3 cross = Cross(v1, v2);
		Matrix4x4 worldMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, rotate, translate);
		Matrix4x4 cameraMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, cameraPosition);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
		// WVPMatrixを作る
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		// ViewportMatrixを作る
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, 1280.0f, 720, 0.0f, 1.0f);
		// Screen空間へと頂点を変換する
		Vector3 screenVertices[3];

		for (uint32_t i = 0; i < 3; ++i)
		{
			// NDC(正規化デバイス座標系)まで変換
			/*
			Transformを使うと同次座標系->デカルト座標系の処理が行われ
			結果的にZDivideが行われることになる
			(遠いほど小さく見えるようにする)
			*/
			Vector3 ndcVertex = Transform(kLocalVertices[i], worldViewProjectionMatrix);
			// Viewport変換を行ってScreen空間へ
			screenVertices[i] = Transform(ndcVertex, viewportMatrix);
		}
		
		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		VectorScreenPrintf(0, 0, cross, "Cross");
		
		// 描画
		// 裏向きの時は表示しない
		if (screenVertices[1].x >= screenVertices[2].x)
		{
			Novice::DrawTriangle(
				int(screenVertices[0].x), int(screenVertices[0].y),
				int(screenVertices[1].x), int(screenVertices[1].y),
				int(screenVertices[2].x), int(screenVertices[2].y),
				RED, kFillModeSolid
			);
		}

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
