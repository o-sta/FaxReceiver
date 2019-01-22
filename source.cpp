//==============================================================================
// Fax Receiver
//==============================================================================
#ifndef UNICODE
#define UNICODE
#endif 
#pragma warning(disable: 4996)		// VisualStudioの警告を無視
//------------------------------------------------------------------------------
// ● 定数の定義
//------------------------------------------------------------------------------
// ボタン
#define BUTTON_START	0			// スタートボタン
#define BUTTON_STOP		1			// ストップボタン
#define BUTTON_CLEAR	2			// クリアボタン
#define BUTTON_SAVE		3			// セーブボタン
#define BUTTON_LOAD		4			// ファイルのロード
#define BUTTON_CHANGE	5			// スレショルド変更ボタン
#define BUTTON_RESET	6			// フィルタリセット
#define BUTTON_FH		7			// 読み取り方向を反転
#define BUTTON_FV		8			// 画像を180度回転
#define BUTTON_TRIM		9			// 画像のトリミング
#define BUTTON_EXEC		10			// フィルタ実行ボタン

// エディタ
#define EDIT_THRESHOLD	11			// スレショルド数値エディタ
#define EDIT_BIN		12			// 二値化のスレショルド
#define EDIT_MEDIAN		13			// メジアンフィルタの係数
#define EDIT_BS_B		14			// 膨張収縮の膨張回数
#define EDIT_BS_S		15			// 膨張収縮の収縮回数
#define EDIT_SB_S		16			// 収縮膨張の収縮回数
#define EDIT_SB_B		17			// 収縮膨張の膨張回数
#define EDIT_GAMMA		102			// ガンマ値補正チェック
#define EDIT_CONTRAST	103			// コントラスト値補正チェック

// チェックボックス
#define CHECKBOX_AUTO	18			// スレショルド自動変更チェック
#define CHECKBOX_BIN	19			// 二値化チェック
#define CHECKBOX_MEDIAN	20			// メジアンチェック
#define CHECKBOX_BS		21			// 膨張縮小チェック
#define CHECKBOX_SB		22			// 縮小膨張チェック
#define CHECKBOX_GAMMA	101			// ガンマ値補正チェック
#define CHECKBOX_CONTRAST	104		// コントラスト値補正チェック

// 状態
#define STATE_STOP		0			// 読み込みが止まっているとき
#define STATE_READING	1			// 読み込みを行っているとき	Z		

// スレショルド関連
#define THRESHOLD_TIMER_ID 1		// スレッショルドタイマーのID識別
#define THRESHOLD_TIMER_TIME 3000	// スレッショルドの更新間隔[ms]

// 読み取り関連
#define BufSiz 1024					// サンプリングバッファ数
#define AM_ave 16					// AM復調時に平均をとる個数
#define Samples_per_sec 22050		// サンプリング周波数
#define Thre_count 20				// 同期を確認するためのAM復調データの個数
#define DotBufSiz 10000				// 1ラインの最大ドット数
#define SIZE_W 420					// 紙のドットサイズ幅 210*2
#define SIZE_H 594					// 紙のドットサイズ高さ 297*2
#define SIZE_MARGIN_H 50			// 紙の余分なドットサイズ高さ

// アクション
#define ACTION_BIN		0			// 二値化
#define ACTION_MEDIAN	1			// メジアンフィルタ
#define ACTION_BS		2			// 膨張収縮フィルタ
#define ACTION_SB		3			// 収縮膨張フィルタ
#define ACTION_SS		4			// 鮮鋭化フィルタ
#define ACTION_GAMMA	5			// ガンマ値補正
#define ACTION_CONTRAST	6			// コントラスト値補正

//------------------------------------------------------------------------------
// ● ライブラリのインクルード
//------------------------------------------------------------------------------
#include <windows.h>				// Windows API
#include <cstdio>
#include <iostream>					// コンソール出力関連
#include <sstream>					// 文字列処理関連
#include <thread>					// マルチスレッド
#include <time.h>					// 時間関連
#include <opencv2/core.hpp>			// coreモジュールのヘッダー
#include <opencv2/highgui.hpp>		// highguiモジュールのヘッダー
#include <opencv2/imgproc/imgproc.hpp>	// opencvモジュール
#pragma comment (lib, "winmm.lib")	// オーディオデバイス関連
#include "libopencv.hpp"
//------------------------------------------------------------------------------
// ● グローバル変数の定義
//------------------------------------------------------------------------------
int Threshold = 67;
int DemMax = 1;
cv::Mat *gpOutImg;					// 表示させる画像の参照を記録しておく
cv::Mat gOutImg(cv::Size(SIZE_W, SIZE_H + SIZE_MARGIN_H), CV_8UC1, 255);	// 読み取り画像
cv::Mat gOutImg_reset(cv::Size(SIZE_W, SIZE_H + SIZE_MARGIN_H), CV_8UC1, 255);	// リセットを押したときの画像
cv::Mat gOutImg_base(cv::Size(SIZE_W, SIZE_H + SIZE_MARGIN_H), CV_8UC1, 255);	// ベースの画像
cv::Mat waveForm(cv::Size(512, 128), CV_8UC3, cv::Scalar(255, 255, 255));	// 波形データ
time_t current_time;				// 取得した時間を記憶する
struct tm *t_st;					// 時間を表示するための構造体
bool gAutoThreshold = false;		// スレッショルドの自動変更
HWND gThEdit;						// 同期信号スレショルド入力エディタのハンドル
HWND gThBin;						// 二値化スレショルド入力エディタのハンドル
HWND gMed;							// 二値化スレショルド入力エディタのハンドル
HWND gBS_B;							// 膨張収縮フィルタの膨張回数
HWND gBS_S;							// 膨張収縮フィルタの収縮回数
HWND gSB_S;							// 収縮収縮フィルタの収縮回数
HWND gSB_B;							// 収縮収縮フィルタの膨張回数
HWND gGamma;						// ガンマ値補正エディタのハンドル
HWND gContrast;						// Contrast補正エディタのハンドル
WINDOWINFO wInfo;					// ウィンドウの情報

//------------------------------------------------------------------------------
// ● プロトタイプ宣言
//------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
void cvOutImage(cv::Mat *img);
void cvWaveForm(cv::Mat *img);
void filteringImage(cv::Mat& input_img, cv::Mat& output_img, int ACTION);
void executionFiltering(HWND hwnd);
void imageTrimming_d(HWND hwnd, int xp);
void saveImage(char *wStrC);
int loadImage(char *wStrC);
void flipVertically(int xp);
void flipHorizontally(int xp);
void enableWindows(HWND hwnd, int action);

//------------------------------------------------------------------------------
// ● メイン関数
//------------------------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout); // 標準出力をコンソールにする
	freopen("CONIN$", "r", stdin);   // 標準入力をコンソールにする
	std::cout << "Fax Receiver v1.02" << std::endl;

	// ウィンドウ クラスを登録する
	const wchar_t CLASS_NAME[] = L"Fax Receiver v1.02";

	WNDCLASS wc = {};

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	// ウィンドウを作成する
	HWND hwnd = CreateWindowEx(
		0,                              // オプションのウィンドウ スタイル
		CLASS_NAME,                     // ウィンドウ クラス
		L"Fax Receiver",				// ウィンドウ テキスト
		WS_OVERLAPPEDWINDOW,            // ウィンドウ スタイル
		40, 40, 500, 385,				// 位置とサイズ
		NULL,							// 親ウィンドウ    
		NULL,							// メニュー
		hInstance,						// インスタンス ハンドル
		NULL							// 追加のアプリケーション データ
	);

	if (hwnd == NULL)
	{
		return 0;
	}

	wInfo.cbSize = sizeof(WINDOWINFO);
	GetWindowInfo(hwnd, &wInfo);

	//ボタン作成
	// スタート
	CreateWindow(
		TEXT("BUTTON"), TEXT("Start"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		275, 5, 100, 30,
		hwnd, (HMENU)BUTTON_START, hInstance, NULL
	);
	// ストップ
	CreateWindow(
		TEXT("BUTTON"), TEXT("Stop"),
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		380, 5, 100, 30,
		hwnd, (HMENU)BUTTON_STOP, hInstance, NULL
	);
	// クリア
	CreateWindow(
		TEXT("BUTTON"), TEXT("Clear"),
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		380, 75, 100, 30,
		hwnd, (HMENU)BUTTON_CLEAR, hInstance, NULL
	);
	// 保存
	CreateWindow(
		TEXT("BUTTON"), TEXT("Save"),
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		380, 40, 100, 30,
		hwnd, (HMENU)BUTTON_SAVE, hInstance, NULL
	);
	// 読み込み
	CreateWindow(
		TEXT("BUTTON"), TEXT("Load"),
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		275, 40, 100, 30,
		hwnd, (HMENU)BUTTON_LOAD, hInstance, NULL
	);
	// スレッショルド変更
	CreateWindow(
		TEXT("BUTTON"), TEXT("Change"),
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		110, 5, 70, 30,
		hwnd, (HMENU)BUTTON_CHANGE, hInstance, NULL
	);
	// スレショルド自動変更チェックボックス
	CreateWindow(
		TEXT("BUTTON"), TEXT("Auto"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		185, 5, 60, 30,
		hwnd, (HMENU)CHECKBOX_AUTO, hInstance, NULL
	);
	// コントラスト値補正チェックボックス
	CreateWindow(
		TEXT("BUTTON"), TEXT("Contrast"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		5, 100 + 35, 220, 30,
		hwnd, (HMENU)CHECKBOX_CONTRAST, hInstance, NULL
	);
	// ガンマ値補正チェックボックス
	CreateWindow(
		TEXT("BUTTON"), TEXT("Gamma"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		5, 135 + 35, 220, 30,
		hwnd, (HMENU)CHECKBOX_GAMMA, hInstance, NULL
	);
	// 二値化(Binarization)チェックボックス
	CreateWindow(
		TEXT("BUTTON"), TEXT("Binarization"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		5, 170 + 35, 220, 30,
		hwnd, (HMENU)CHECKBOX_BIN, hInstance, NULL
	);
	// メジアンフィルタ(Median filter)チェックボックス
	CreateWindow(
		TEXT("BUTTON"), TEXT("Median"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		5, 205 + 35, 220, 30,
		hwnd, (HMENU)CHECKBOX_MEDIAN, hInstance, NULL
	);

	// 膨張収縮フィルタ(Expansion and contraction filter)チェックボックス
	CreateWindow(
		TEXT("BUTTON"), TEXT("Expansion and contraction"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		5, 240 + 35, 220, 30,
		hwnd, (HMENU)CHECKBOX_BS, hInstance, NULL
	);

	// 収縮膨張フィルタ(Contraction and expansion filter)チェックボックス
	CreateWindow(
		TEXT("BUTTON"), TEXT("Contraction and expansion"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		5, 275 + 35, 220, 30,
		hwnd, (HMENU)CHECKBOX_SB, hInstance, NULL
	);
	// リセットボタン
	CreateWindow(
		TEXT("BUTTON"), TEXT("Reset"),
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		380, 135 + 35, 100, 30,
		hwnd, (HMENU)BUTTON_RESET, hInstance, NULL
	);
	// 左右反転ボタン
	CreateWindow(
		TEXT("BUTTON"), TEXT("Flip H"),
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		380, 170 + 35, 100, 30,
		hwnd, (HMENU)BUTTON_FH, hInstance, NULL
	);
	// 上下反転ボタン
	CreateWindow(
		TEXT("BUTTON"), TEXT("Flip V"),
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		380, 205 + 35, 100, 30,
		hwnd, (HMENU)BUTTON_FV, hInstance, NULL
	);
	// トリミング実行ボタン
	CreateWindow(
		TEXT("BUTTON"), TEXT("Trimming"),
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		380, 240 + 35, 100, 30,
		hwnd, (HMENU)BUTTON_TRIM, hInstance, NULL
	);
	// フィルタ実行ボタン
	CreateWindow(
		TEXT("BUTTON"), TEXT("Execution"),
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		380, 275 + 35, 100, 30,
		hwnd, (HMENU)BUTTON_EXEC, hInstance, NULL
	);
	// ウィンドウ表示
	EnableWindow(GetDlgItem(hwnd, BUTTON_STOP), FALSE);
	ShowWindow(hwnd, nCmdShow);

	// ポインタ設定
	gpOutImg = &gOutImg;

	// マルチスレッド起動
	std::thread oi(cvOutImage, gpOutImg);
	std::thread wf(cvWaveForm, &waveForm);

	// メッセージ処理
	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	// マルチスレッドの解放
	oi.detach();
	wf.detach();
	return 0;
}
//------------------------------------------------------------------------------
// ● ウィンドウプロシージャ
//------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	// static変数
	static WAVEFORMATEX wfe;
	static HWAVEIN hWaveIn;
	static BYTE *bWave;
	static WAVEHDR whdr;
	static WAVEHDR *p_whdr[2];
	static int whdrID = 0;				//使用しているヘッダのインデックス
	static int Demodulated[BufSiz / AM_ave + 1];
	static int DotBuf[DotBufSiz];
	static int xp;
	static int count_th = 0, up = 0, dc = 0;
	// ファイル関連 ※廃止予定
	static OPENFILENAME ofn = { 0 };
	static TCHAR strCustom[256] = TEXT("Before files\0*.*\0\0"), strFile[MAX_PATH];
	static char wStrC[260];
	static size_t wLen = 0;

	//static HWND thEdit;

	// ローカル変数
	DWORD FBlockSize = BufSiz;
	DWORD len;
	int sum_data = 0;
	int i, j, no_am, y1 = 0, y2 = 0, y3 = 0;
	const unsigned char* data;
	double s, ck;
	int c;
	std::ostringstream filename;
	wchar_t thValue[4];
	int tmp;

	switch (uMsg)
	{
	case WM_DESTROY:	// ウィンドウが消えたとき
		free(bWave);
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:		// ウィンドウが描画されたとき
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
		EndPaint(hwnd, &ps);
		return 0;
	}
	case WM_CREATE:		// ウィンドウが生成されたとき
						// サウンドデバイス関連
		std::cout << "WM_CREATE" << std::endl;
		wfe.wFormatTag = WAVE_FORMAT_PCM;
		wfe.nChannels = (WORD)(1);
		wfe.nSamplesPerSec = (DWORD)(Samples_per_sec);
		wfe.wBitsPerSample = (WORD)(8);
		wfe.nBlockAlign = (WORD)((wfe.wBitsPerSample / 8)*wfe.nChannels);
		wfe.nAvgBytesPerSec = (DWORD)wfe.nSamplesPerSec*wfe.nBlockAlign;
		// ファイル読み込み関連
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = hwnd;
		ofn.lpstrFilter = TEXT("PNG files {*.png}\0*.png\0")
			TEXT("All files {*.*}\0*.*\0\0");
		ofn.lpstrCustomFilter = strCustom;
		ofn.nMaxCustFilter = 256;
		ofn.nFilterIndex = 0;
		ofn.lpstrFile = strFile;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_FILEMUSTEXIST;


		// 同期信号用スレショルドエディタ
		gThEdit = CreateWindow(
			TEXT("EDIT"), TEXT("65"),
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
			5, 5, 100, 30,
			hwnd, (HMENU)EDIT_THRESHOLD, ((LPCREATESTRUCT)(lParam))->hInstance, NULL
		);
		// コントラスト値変更エディタ
		gContrast = CreateWindow(
			TEXT("EDIT"), TEXT("1.0"),
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
			235, 102 + 35, 45, 26,
			hwnd, (HMENU)EDIT_CONTRAST, ((LPCREATESTRUCT)(lParam))->hInstance, NULL
		);
		// ガンマ値変更エディタ
		gGamma = CreateWindow(
			TEXT("EDIT"), TEXT("1.0"),
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
			235, 137 + 35, 45, 26,
			hwnd, (HMENU)EDIT_GAMMA, ((LPCREATESTRUCT)(lParam))->hInstance, NULL
		);
		
		// 二値化スレショルドエディタ
		gThBin = CreateWindow(
			TEXT("EDIT"), TEXT("128"),
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
			235, 172 + 35, 45, 26,
			hwnd, (HMENU)EDIT_BIN, ((LPCREATESTRUCT)(lParam))->hInstance, NULL
		);
		// メジアンフィルタのパラメータエディタ
		gMed = CreateWindow(
			TEXT("EDIT"), TEXT("3"),
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
			235, 207 + 35, 45, 26,
			hwnd, (HMENU)EDIT_MEDIAN, ((LPCREATESTRUCT)(lParam))->hInstance, NULL
		);
		// 膨張収縮フィルタ
		gBS_B = CreateWindow(
			TEXT("EDIT"), TEXT("1"),
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
			235, 242 + 35, 45, 26,
			hwnd, (HMENU)EDIT_BS_B, ((LPCREATESTRUCT)(lParam))->hInstance, NULL
		);
		// 膨張収縮フィルタ
		gBS_S = CreateWindow(
			TEXT("EDIT"), TEXT("1"),
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
			285, 242 + 35, 45, 26,
			hwnd, (HMENU)EDIT_BS_S, ((LPCREATESTRUCT)(lParam))->hInstance, NULL
		);
		// 膨張収縮フィルタ
		gSB_S = CreateWindow(
			TEXT("EDIT"), TEXT("1"),
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
			235, 277 + 35, 45, 26,
			hwnd, (HMENU)EDIT_SB_S, ((LPCREATESTRUCT)(lParam))->hInstance, NULL
		);
		// 膨張収縮フィルタ
		gSB_B = CreateWindow(
			TEXT("EDIT"), TEXT("1"),
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
			285, 277 + 35, 45, 26,//285, 275, 45, 30,
			hwnd, (HMENU)EDIT_SB_B, ((LPCREATESTRUCT)(lParam))->hInstance, NULL
		);
		return 0;

	case WM_COMMAND:	// Buttonによる入力が行われたとき
		switch (LOWORD(wParam)) {
		case BUTTON_START:		// 読み取り開始
			if (waveInOpen(&hWaveIn, WAVE_MAPPER, &wfe, (DWORD_PTR)hwnd, 0, CALLBACK_WINDOW) == MMSYSERR_NOERROR) {
				std::cout << "Start : " << std::endl;
			}
			break;
		case BUTTON_STOP:		// 読み取り停止
			waveInReset(hWaveIn);
			for (i = 0; i<2; i++) {
				waveInUnprepareHeader(hWaveIn, p_whdr[i], sizeof(WAVEHDR));
				free(p_whdr[i]->lpData);
				free(p_whdr[i]);
			}
			whdrID = 0;
			waveInClose(hWaveIn);
			hWaveIn = 0;
			break;
		case BUTTON_CLEAR:		// 全画面クリア
			std::cout << "Cleared image" << std::endl;
			gOutImg_base.copyTo(gOutImg);
			gOutImg.copyTo(gOutImg_reset);
			cv::imshow("Image", gOutImg);
			xp = 0;
			break;
		case BUTTON_SAVE:		// 画像の保存
			if (GetSaveFileName(&ofn)) {
				setlocale(LC_ALL, "japanese");//ロケール指定
				wcstombs_s(&wLen, wStrC, 260, strFile, _TRUNCATE); //文字変換
				saveImage(wStrC);
			}
			break;
		case BUTTON_CHANGE:			// スレッショルド変更
			GetWindowText(gThEdit, thValue, 4);
			tmp = _wtoi(thValue);
			if (tmp >= 0 && tmp <= 128) {
				Threshold = tmp;
				std::cout << "Threshold -> " << Threshold << std::endl;
			}
			else {
				std::cout << "Failed : Change Threshold" << std::endl;
			}

			break;
		case CHECKBOX_AUTO:			// 自動スレッショルド変更スイッチ
			if (IsDlgButtonChecked(hwnd, CHECKBOX_AUTO) == BST_CHECKED) {
				EnableWindow(GetDlgItem(hwnd, BUTTON_CHANGE), FALSE);
				EnableWindow(gThEdit, FALSE);
				gAutoThreshold = true;
			}
			else {
				EnableWindow(GetDlgItem(hwnd, BUTTON_CHANGE), TRUE);
				EnableWindow(gThEdit, TRUE);
				gAutoThreshold = false;
			}
			break;
		case BUTTON_EXEC:			// フィルタ実行
			executionFiltering(hwnd);
			break;
		case BUTTON_LOAD:			// 画像読み込み
			if (GetOpenFileName(&ofn)) {
				setlocale(LC_ALL, "japanese");//ロケール指定
				wcstombs_s(&wLen, wStrC, 260, strFile, _TRUNCATE); //文字変換
				tmp = loadImage(wStrC);
				if (!(tmp == -1)) {
					enableWindows(hwnd, STATE_STOP);
					xp = tmp;
				}
			}
			break;
		case BUTTON_TRIM:			// トリミング
			imageTrimming_d(hwnd, xp);
			break;
		case BUTTON_FV:				// 180度回転
			flipVertically(xp);
			break;
		case BUTTON_FH:				// 左右反転
			flipHorizontally(xp);
			break;
		case BUTTON_RESET:			// すべてのフィルタ効果を消去
			gOutImg_reset.copyTo(gOutImg);
			cv::imshow("Image", gOutImg);
			enableWindows(hwnd, STATE_STOP);
			std::cout << "Reset : Image" << std::endl;
			break;
		}
		return 0;
	case MM_WIM_OPEN:	// サウンドデバイスが開いたとき
		gOutImg_reset.copyTo(gOutImg);
		cv::imshow("Image", gOutImg);
		for (i = 0; i < 2; i++) {
			p_whdr[i] = (WAVEHDR*)calloc(1, sizeof(WAVEHDR));
			p_whdr[i]->lpData = (char*)malloc(FBlockSize);
			p_whdr[i]->dwBufferLength = FBlockSize;
			if (waveInPrepareHeader(hWaveIn, p_whdr[i], sizeof(WAVEHDR)) == MMSYSERR_NOERROR) {
			}
			else {
			}
			if (waveInAddBuffer(hWaveIn, p_whdr[i], sizeof(WAVEHDR)) == MMSYSERR_NOERROR) {
			}
			else {
			}
		}
		waveInStart(hWaveIn);
		SetTimer(hwnd, THRESHOLD_TIMER_ID, THRESHOLD_TIMER_TIME, TimerProc);
		enableWindows(hwnd, STATE_READING);
		return 0;

	case MM_WIM_DATA:	// サウンドデバイスのバッファにデータが溜まった場合
	{
		if (hWaveIn != 0) {		// Stopが押されたとき、解放後のhdrポインタにアクセスするのを防止
			WAVEHDR *hdr = p_whdr[whdrID];
			whdrID ^= 1;
			len = hdr->dwBytesRecorded;
			if (len > 0) {

				// AM復調
				data = (unsigned char*)hdr->lpData;
				j = 0;
				no_am = 0;
				do {
					for (i = 0; i < AM_ave; i++) {
						Demodulated[no_am] += abs(128 - data[j++]);
					}
					Demodulated[no_am] /= AM_ave;
					if (DemMax < Demodulated[no_am]) DemMax = Demodulated[no_am];
					no_am++;
				} while (j < BufSiz);

				// 波形を描画
				waveForm = cv::Scalar::all(255);
				for (i = 0; i<512; i++) {
					y1 = (int)(data[i] >> 1);
					if (i%AM_ave == 0) y2 = (128 - Demodulated[i / AM_ave]) >> 1;
					y3 = (128 - Threshold) >> 1;
					if (y2 <= 0) y2 = 0;
					waveForm.at<cv::Vec3b>(y1, i)[0] = 0;
					waveForm.at<cv::Vec3b>(y1, i)[1] = 0;
					waveForm.at<cv::Vec3b>(y1, i)[2] = 0;
					waveForm.at<cv::Vec3b>(y2, i)[0] = 0;
					waveForm.at<cv::Vec3b>(y2, i)[1] = 0;
					waveForm.at<cv::Vec3b>(y2, i)[2] = 255;
					waveForm.at<cv::Vec3b>(y3, i)[0] = 255;
					waveForm.at<cv::Vec3b>(y3, i)[1] = 0;
					waveForm.at<cv::Vec3b>(y3, i)[2] = 0;
				}
				cv::imshow("Wave", waveForm);
			}

			waveInPrepareHeader(hWaveIn, hdr, sizeof(WAVEHDR));
			waveInAddBuffer(hWaveIn, hdr, sizeof(WAVEHDR));

			// スレショルドの確認と画像描画
			ck = 255 / Threshold;
			for (i = 0; i < no_am; i++) {
				if (up == 0) {
					if (Demodulated[i] < Threshold) {
						count_th++;
						if (count_th > Thre_count) {
							count_th = 0;
							up = 1;
							// 描画を開始する
							if (xp < SIZE_W / 2) {
								s = dc / (double)(SIZE_H + SIZE_MARGIN_H);
								for (j = 0; j < SIZE_H + SIZE_MARGIN_H; j++) {
									c = (int)(ck*Demodulated[(int)(s*j)]);
									if (c > 255) c = 255;
									gOutImg.data[j * gOutImg.cols + xp * 2] = c;
									gOutImg.data[j * gOutImg.cols + xp * 2 + 1] = c;
								}
								cv::imshow("Image", *gpOutImg);
								if (xp < SIZE_W / 2) xp++;
								std::cout << "data " << (int)(s*j) << std::endl;
							}
							dc = 0;
						}
					}
					else {
						count_th = 0;
					}
				}
				if (up == 1) {
					if (Demodulated[i] >= Threshold) {
						count_th++;
						if (count_th > Thre_count) {
							count_th = 0;
							up = 0;
						}
					}
					else {
						count_th = 0;
					}
				}
				if (dc < DotBufSiz) DotBuf[dc++] = Demodulated[i];
			}
		}
	}
	return 0;

	case MM_WIM_CLOSE:	// オーディオを閉じたとき
		if (KillTimer(hwnd, THRESHOLD_TIMER_ID)) {
		}
		enableWindows(hwnd, STATE_STOP);
		gOutImg.copyTo(gOutImg_reset);
		std::cout << "Stop : Location x = " << xp << std::endl;
		return 0;

	case WM_CLOSE:		// ウィンドウを閉じる命令を行ったとき
		DestroyWindow(hwnd);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


//------------------------------------------------------------------------------
// ● スレッショルド変更のコールバック関数
//------------------------------------------------------------------------------
void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	wchar_t str[4];
	if (gAutoThreshold) {
		Threshold = (int)((double)DemMax * 0.5);
		if (Threshold<1) Threshold = 1;
		DemMax = 1;
		_itow(Threshold, str, 10);
		SetWindowText(gThEdit, str);
		std::cout << "Auto : Threshold -> " << Threshold << std::endl;
	}
}
//------------------------------------------------------------------------------
// ● 画像出力用マルチスレッド
//------------------------------------------------------------------------------
void cvOutImage(cv::Mat *img) {
	cv::namedWindow("Image", cv::WINDOW_AUTOSIZE);
	cv::moveWindow("Image", wInfo.rcClient.right + 50, wInfo.rcClient.top - 31);
	cv::imshow("Image", *img);
	while (true) {
		cv::waitKey(0);
	}
}
//------------------------------------------------------------------------------
// ● 波形表示用マルチスレッド
//------------------------------------------------------------------------------
void cvWaveForm(cv::Mat *img) {
	cv::namedWindow("Wave", cv::WINDOW_AUTOSIZE);
	cv::moveWindow("Wave", wInfo.rcClient.left - 8, wInfo.rcClient.bottom + 50);
	cv::imshow("Wave", *img);
	while (true) {
		cv::waitKey(0);
	}
}
//------------------------------------------------------------------------------
// ● フィルタ処理
//------------------------------------------------------------------------------
void filteringImage(cv::Mat& input_img, cv::Mat& output_img, int ACTION) {
	
	wchar_t str[4];				//文字読み込み一次格納
	int bin_threshold = 200;	//2値化の閾値　//閾値 > 画素値　-> 黒(値は0), else -> 白(値は255)
	int mediun_size = 3;		//メディアンフィルタ
	int bocho_n = 1;			//膨張処理回数
	int shukusho_n = 1;			//縮小処理回数
	const float k = -1.0;		//鮮鋭化フィルタ
	cv::Mat sharpningKernel4 = (cv::Mat_<float>(3, 3) << 0.0, k, 0.0, k, 5.0, k, 0.0, k, 0.0);
	cv::Mat sharpningKernel8 = (cv::Mat_<float>(3, 3) << k, k, k, k, 9.0, k, k, k, k);
	double gamma = 0.2;			// ガンマ値補正
	uchar lut[256];
	double gm;
	cv::Mat p = input_img.reshape(0, 1).clone();
	int i;
	double a = 1;				// コントラスト値補正
	//ACTIONに設定した動作を行い出力画像を返す
	switch (ACTION) {
	case ACTION_BIN: {
		GetWindowText(gThBin, str, 4);
		bin_threshold = _wtoi(str);
		if (!(bin_threshold > 0 && bin_threshold < 255)) {
			bin_threshold = 128;
			_itow(bin_threshold, str, 10);
			SetWindowText(gThBin, str);
		}
		cv::threshold(input_img, output_img, bin_threshold, 255, cv::THRESH_BINARY);
		break;
	}
	case ACTION_MEDIAN: {
		GetWindowText(gMed, str, 4);
		mediun_size = _wtoi(str);
		if (!(mediun_size > 1 && mediun_size % 2 == 1)) {
			mediun_size = 3;
			_itow(mediun_size, str, 10);
			SetWindowText(gMed, str);
		}
		cv::medianBlur(input_img, output_img, mediun_size);
		break;
	}
	case ACTION_BS: {
		GetWindowText(gBS_B, str, 4);
		bocho_n = _wtoi(str);
		if (!(bocho_n > 1)) {
			bocho_n = 1;
			_itow(bocho_n, str, 10);
			SetWindowText(gBS_B, str);
		}
		GetWindowText(gBS_S, str, 4);
		shukusho_n = _wtoi(str);
		if (!(shukusho_n > 1)) {
			shukusho_n = 1;
			_itow(shukusho_n, str, 10);
			SetWindowText(gBS_S, str);
		}
		cv::erode(input_img, output_img, cv::Mat(), cv::Point(-1, -1), bocho_n);
		cv::dilate(output_img, output_img, cv::Mat(), cv::Point(-1, -1), shukusho_n);
		break;
	}
	case ACTION_SB: {
		GetWindowText(gSB_S, str, 4);
		bocho_n = _wtoi(str);
		if (!(bocho_n > 1)) {
			bocho_n = 1;
			_itow(bocho_n, str, 10);
			SetWindowText(gSB_S, str);
		}
		GetWindowText(gSB_B, str, 4);
		shukusho_n = _wtoi(str);
		if (!(shukusho_n > 1)) {
			shukusho_n = 1;
			_itow(shukusho_n, str, 10);
			SetWindowText(gSB_B, str);
		}
		cv::dilate(input_img, output_img, cv::Mat(), cv::Point(-1, -1), shukusho_n);
		cv::erode(output_img, output_img, cv::Mat(), cv::Point(-1, -1), bocho_n);
		break;
	}
	case ACTION_SS: {
		cv::filter2D(input_img, output_img, input_img.depth(), sharpningKernel4);
		break;
	}
	case ACTION_GAMMA: {
		GetWindowText(gGamma, str, 4);
		gamma = _wtof(str);
		gm = 1.0 / gamma;
		std::cout << gamma << std::endl;
		for (i = 0; i < 256; i++) {
			lut[i] = (uchar)(pow(1.0*(double)i / 255.0, gm) * 255);
		}
		// 輝度値の置き換え処理
		 cv::LUT(input_img, cv::Mat(cv::Size(256, 1), CV_8U, lut), output_img);
	}
	case ACTION_CONTRAST: {
		GetWindowText(gContrast, str, 4);
		a = _wtof(str);
		// ルックアップテーブル作成
		for (int i = 0; i < 256; i++) {
			lut[i] = 255.0 / (1 + exp(-a * (double)(i - 128) / 255.0));
		}
		cv::LUT(input_img, cv::Mat(cv::Size(256, 1), CV_8U, lut), output_img);
	}
	}
}

//------------------------------------------------------------------------------
// ● フィルタの判定
//------------------------------------------------------------------------------
void executionFiltering(HWND hwnd) {
	if (IsDlgButtonChecked(hwnd, CHECKBOX_BIN) == BST_CHECKED) {
		filteringImage(gOutImg, gOutImg, ACTION_BIN);
	}
	if (IsDlgButtonChecked(hwnd, CHECKBOX_MEDIAN) == BST_CHECKED) {
		filteringImage(gOutImg, gOutImg, ACTION_MEDIAN);
	}
	if (IsDlgButtonChecked(hwnd, CHECKBOX_BS) == BST_CHECKED) {
		filteringImage(gOutImg, gOutImg, ACTION_BS);
	}
	if (IsDlgButtonChecked(hwnd, CHECKBOX_SB) == BST_CHECKED) {
		filteringImage(gOutImg, gOutImg, ACTION_SB);
	}
	if (IsDlgButtonChecked(hwnd, CHECKBOX_GAMMA) == BST_CHECKED) {
		filteringImage(gOutImg, gOutImg, ACTION_GAMMA);
	}
	if (IsDlgButtonChecked(hwnd, CHECKBOX_CONTRAST) == BST_CHECKED) {
		filteringImage(gOutImg, gOutImg, ACTION_CONTRAST);
	}
	cv::imshow("Image", gOutImg);
}

//------------------------------------------------------------------------------
// ● 画像の端検出とトリミング
//------------------------------------------------------------------------------
void imageTrimming_d(HWND hwnd, int xp) {
	int x;
	int y = 0;
	int flag = 0;
	int yLineU = -1;
	int yLineD = -1;
	int yData[SIZE_H + SIZE_MARGIN_H];
	int yDataD[SIZE_H + SIZE_MARGIN_H];
	int tmp, yMax, yNum;
	cv::Mat inputImg = gOutImg;
	cv::Mat outputImg;
	cv::Mat tmpM;

	cv::threshold(inputImg, tmpM, 60, 255, cv::THRESH_BINARY);

	for (y = 0; y < SIZE_H + SIZE_MARGIN_H; y++) {
		tmp = 0;
		for (x = 0; x < xp + 1; x++) {
			tmp += static_cast<int>(tmpM.at<unsigned char>(y, x));
		}
		tmp /= (xp + 1);
		yData[y] = tmp;
	}

	//信号の微分
	for (y = 1; y < SIZE_H + SIZE_MARGIN_H; y++) {
		yDataD[y] = yData[y] - yData[y - 1];
	}

	//画像上部の端検出
	yMax = 0;
	for (y = 1; y < (SIZE_H + SIZE_MARGIN_H) / 4; y++) {
		if (yDataD[y] > yMax) {
			yMax = yDataD[y];
			yNum = y;
			std::cout << yMax << std::endl;
		}
	}
	if (yMax == 0) {
		std::cout << "failed : trimming" << yLineU << " :: " << yLineD << std::endl;
		return;
	};
	tmp = 0;
	for (y = yNum; y < (SIZE_H + SIZE_MARGIN_H) / 2; y++) {
		if (yData[y] > 250) {
			tmp++;
		}
		if (tmp > 0) {
			yLineU = y;
			break;
		}
	}
	//画像下部の端検出
	yMax = 0;
	for (y = SIZE_H + SIZE_MARGIN_H - 1; y >= (SIZE_H + SIZE_MARGIN_H) / 4 * 3; y--) {
		if (yDataD[y] < yMax) {
			yMax = yDataD[y];
			yNum = y;
			std::cout << yMax << std::endl;
		}
	}
	if (yMax == 0) {
		std::cout << "failed : trimming" << yLineU << " :: " << yLineD << std::endl;
		return;
	};
	tmp = 0;
	for (y = yNum; y > (SIZE_H + SIZE_MARGIN_H) / 2; y--) {
		if (yData[y]  > 250) {
			tmp++;
		}
		if (tmp > 0) {
			yLineD = y;
			break;
		}
	}
	if (!(yLineU == -1 || yLineD == -1)) {
		std::cout << "success : trimming" << yLineU << " :: " << yLineD << std::endl;
		cv::Mat outputImg(gOutImg, cv::Rect(0, yLineU, xp * 2, yLineD - yLineU));
		outputImg.copyTo(gOutImg);
		cv::imshow("Image", gOutImg);
		EnableWindow(GetDlgItem(hwnd, BUTTON_TRIM), FALSE);
	}
	else {
		std::cout << "failed : trimming" << yLineU << " :: " << yLineD << std::endl;
	}
}

//------------------------------------------------------------------------------
// ● 画像の保存
//------------------------------------------------------------------------------
void saveImage(char *wStrC) {
	std::string p = wStrC;
	std::stringstream saveFileName;
	std::string file = wStrC;//fullpath.size() = 14
	int path_i = (int)file.find_last_of("\\") + 1;
	int ext_i = (int)file.find_last_of(".");
	std::string path = file.substr(0, path_i);
	std::string fileName = file.substr(path_i, ext_i - path_i);
	saveFileName << path << fileName << ".png";
	std::cout << "Saved : \"" << saveFileName.str() << "\"" << std::endl;
	cv::imwrite(saveFileName.str(), gOutImg);
}

//------------------------------------------------------------------------------
// ● 画像の読み込み
//------------------------------------------------------------------------------
int loadImage(char *wStrC) {
	int xp;
	cv::Mat tmp;
	cv::Mat mat = (cv::Mat_<double>(2, 3) << 1.0, 0.0, 0, 0.0, 1.0, 0);//回転行列

	tmp = cv::imread(wStrC, 0);
	if (tmp.empty()) {
		std::cout << "Failed : Load \"" << wStrC << "\"" << std::endl;
		return -1;
	}
	xp = tmp.cols / 2;
	cv::warpAffine(tmp, gOutImg, mat, gOutImg.size(), CV_INTER_LINEAR, cv::BORDER_CONSTANT, 255);
	gOutImg.copyTo(gOutImg_reset);
	cv::imshow("Image", gOutImg);
	std::cout << "Load : \"" << wStrC << "\"" << std::endl;
	std::cout << "Location x : " << xp << std::endl;
	return xp;
}

//------------------------------------------------------------------------------
// ● 180度回転処理
//------------------------------------------------------------------------------
void flipVertically(int xp) {
	cv::Mat tmp = gOutImg;
	cv::Mat mat = (cv::Mat_<double>(2, 3) << -1.0, 0.0, xp * 2, 0.0, -1.0, SIZE_H + SIZE_MARGIN_H);//回転行列
	cv::warpAffine(tmp, gOutImg, mat, gOutImg.size(), CV_INTER_LINEAR, cv::BORDER_CONSTANT, 255);
	cv::imshow("Image", gOutImg);
}

//------------------------------------------------------------------------------
// ● 反転処理(縦軸中心)
//------------------------------------------------------------------------------
void flipHorizontally(int xp) {
	cv::Mat tmp = gOutImg;
	cv::Mat mat = (cv::Mat_<double>(2, 3) << -1.0, 0.0, xp * 2, 0.0, 1.0, 0);//回転行列
	cv::warpAffine(tmp, gOutImg, mat, gOutImg.size(), CV_INTER_LINEAR, cv::BORDER_CONSTANT, 255);
	cv::imshow("Image", gOutImg);
}

//------------------------------------------------------------------------------
// ● ウィンドウの有効、無効設定
//------------------------------------------------------------------------------
void enableWindows(HWND hwnd, int action) {
	action ? action = 1 : action = 0;
	//SD_READING (action = 1)のとき !action は 0 = FALSE になる
	EnableWindow(GetDlgItem(hwnd, BUTTON_START), !action);
	EnableWindow(GetDlgItem(hwnd, BUTTON_STOP), action);
	EnableWindow(GetDlgItem(hwnd, BUTTON_LOAD), !action);
	EnableWindow(GetDlgItem(hwnd, BUTTON_RESET), !action);
	EnableWindow(GetDlgItem(hwnd, BUTTON_FV), !action);
	EnableWindow(GetDlgItem(hwnd, BUTTON_FH), !action);
	EnableWindow(GetDlgItem(hwnd, BUTTON_TRIM), !action);
	EnableWindow(GetDlgItem(hwnd, BUTTON_EXEC), !action);
	EnableWindow(GetDlgItem(hwnd, EDIT_BIN), !action);
	EnableWindow(GetDlgItem(hwnd, EDIT_MEDIAN), !action);
	EnableWindow(GetDlgItem(hwnd, EDIT_BS_B), !action);
	EnableWindow(GetDlgItem(hwnd, EDIT_BS_S), !action);
	EnableWindow(GetDlgItem(hwnd, EDIT_SB_S), !action);
	EnableWindow(GetDlgItem(hwnd, EDIT_SB_B), !action);
	EnableWindow(GetDlgItem(hwnd, EDIT_GAMMA), !action);
	EnableWindow(GetDlgItem(hwnd, EDIT_CONTRAST), !action);
	EnableWindow(GetDlgItem(hwnd, CHECKBOX_BIN), !action);
	EnableWindow(GetDlgItem(hwnd, CHECKBOX_MEDIAN), !action);
	EnableWindow(GetDlgItem(hwnd, CHECKBOX_BS), !action);
	EnableWindow(GetDlgItem(hwnd, CHECKBOX_SB), !action);
	EnableWindow(GetDlgItem(hwnd, CHECKBOX_GAMMA), !action);
	EnableWindow(GetDlgItem(hwnd, CHECKBOX_CONTRAST), !action);
}