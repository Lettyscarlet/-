#include <TimerThree.h>

#define UPLIMIT 10 //上限
#define RANDOM 10 //確率

bool sw1 = 0; //左のボタン
bool sw2 = 0; //真ん中のボタン
bool sw3 = 0; //右のボタン
bool swamu = 0; //アーム降下上限検出スイッチ

bool sw1f = 0;
bool sw2f = 0;
bool sw3f = 0;
bool swamuf = 0;

bool pi_oku = 0; //フォトインタラプタY軸奥
bool pi_temae = 0; //フォトインタラプタY軸手前
bool pi_r = 0; //フォトインタラプタX軸右
bool pi_l = 0; //フォトインタラプタX軸左

bool pi_okuf = 0;
bool pi_temaef = 0;
bool pi_rf = 0;
bool pi_lf = 0;

bool nextf = 0;

int co_ue = 0; //アームを下した数を数える用
int co = 0; //カウント用
int co_random = 0; //乱数受け取り用
int co_limit = 0; //回数上限カウント
int tc = 0; //時間カウント
int ddc = 0; //swのチャタリング防止用
int mode = 0; //モード

void setup() {
  pinMode(42, OUTPUT); //モーター右
  pinMode(43, OUTPUT); //モーター左
  pinMode(44, OUTPUT); //モーター奥
  pinMode(45, OUTPUT); //モーター手前
  pinMode(46, OUTPUT); //モーター下降
  pinMode(47, OUTPUT); //モーター上昇

  pinMode(4, INPUT); //アーム奥センサ
  pinMode(5, INPUT); //アーム手前センサ
  pinMode(6, INPUT); //アーム右センサ
  pinMode(7, INPUT); //アーム左センサ

  pinMode(8, INPUT_PULLUP); //ボタン１
  pinMode(9, INPUT_PULLUP); //ボタン２
  pinMode(10, INPUT_PULLUP); //ボタン３
  pinMode(11, INPUT_PULLUP); //アーム下降上限
  pinMode(12, OUTPUT); //アーム開閉

  sw1 = digitalRead(8);
  sw2 = digitalRead(9);
  sw3 = digitalRead(10);
  swamu = digitalRead(11);

  Timer3.initialize(1000);
  Timer3.attachInterrupt(interrupt);
  Serial.begin(9600);
}

void interrupt ()
{
  tc++;
  ddc++;

  //チャタリング防止
  if ( ddc >= 5 )
  {
    ddc = 0;
    sw1 = digitalRead(8);
    sw2 = digitalRead(9);
    sw3 = digitalRead(10);
    swamu = digitalRead(11);
  }
}

void loop()
{
  pi_oku = digitalRead(4);
  pi_temae = digitalRead(5);
  pi_r = digitalRead(6);
  pi_l = digitalRead(7);

  switch (mode)
  {
    case 0: //最初の起動

      //3つのボタン同時押し（1秒以上）から原点に復帰する
      if ( sw1 == 0 && sw2 == 0 && sw3 == 0 && nextf == 0 )
      {
        nextf = 1;
        tc = 0;
      }
      else if ( sw1 == 1 && sw2 == 1 && sw3 == 1 && nextf == 1 )
      {
        nextf = 0;
      }
      if ( nextf == 1 )
      {
        if ( tc > 1000 )
        {
          mode = 6;
        }
      }
      break;
    case 1: //右側に移動する

      if ( sw1 == 0 )
      {
        if ( pi_r == 1 ) nextf = 1;
        sw1f = 1;
        migi();
      }
      if ( sw1 == 1 && sw1f == 1 ) nextf = 1;
      if ( nextf == 1 )
      {
        mode = 2;
        nextf = 0;
        sw1f = 0;
        tomeru();
      }

      //右のボタンを１秒以上押すとメンテンナンスモードに移行
      if ( sw3 == 0 && sw3f == 0 )
      {
        sw3f = 1;
        tc = 0;
      }
      else if ( sw3 == 1 && sw3f == 1 )
      {
        sw3f = 0;
      }
      if ( sw3f == 1 ) {
        if ( tc > 1000 )
        {
          sw3f = 0;
          kaihei();
          tomeru();
          mode = 100;
        }
      }

      break;
    case 2: //奥側に移動する

      if ( sw2 == 0 )
      {
        if ( pi_oku == 1 ) nextf = 1;
        sw2f = 1;
        oku();
      }
      if ( sw2 == 1 && sw2f == 1 ) nextf = 1;
      if ( nextf == 1 )
      {
        mode = 3;
        nextf = 0;
        sw2f = 0;
        tomeru();
        co_ue = 0;
        delay(500);
      }
      break;
    case 3: //アーム降下

      if ( swamu == 1  || sw3 == 0 ) swamuf = 1;
      if ( swamuf == 0 )
      {
        co_ue++;
        orosu();
      }
      else
      {
        tomeru();
        delay(500);
        toziru();
        mode = 4;
        swamuf = 0;
        delay(500);
      }
      break;
    case 4: //アーム上昇

      ageru();
      if ( co_ue * 1.1 < co )
      {
        tomeru();
        mode = 5;
        co = 0;
        co_ue = 0;
      }
      co++;
      break;
    case 5: //確率判定

      //確率を検出
      co_random = random(RANDOM);

      //確率 or 上限が来たら次へ
      if ( co_random == 7 || co_limit == UPLIMIT )
      {
        co_limit = 0;
        mode = 6;
      }

      //確率 or 上限ではない場合は景品を落として次へ
      else
      {
        delay(200);
        hiraku();
        delay(200);
        toziru();
        mode = 6;
        co_limit++;
      }
      break;
    case 6: //原点に戻す（手前）

      if ( pi_temae == 1 ) pi_temaef = 1;
      if ( pi_temaef == 0 )
      {
        temae();
      }
      else
      {
        tomeru();
        oku();
        delay(600);
        tomeru();
        pi_temaef = 0;
        mode = 7;
      }
      break;
    case 7: //原点に戻す（左）

      if ( pi_l == 1 ) pi_lf = 1;
      if ( pi_lf == 0 )
      {
        hidari();
      }
      else
      {
        tomeru();
        migi();
        delay(400);
        tomeru();
        pi_lf = 0;

        //最初の原点復帰なのか動作中の原点に戻すのか
        if ( nextf == 0 )
        {
          mode = 8;
        }
        else
        {
          mode = 1;
          nextf = 0;
        }
        delay(500);
      }
      break;
    case 8: //アイテム落とし

      hiraku();
      delay(500);
      kaihei();
      tomeru();
      mode = 1;
      break;
    case 100: //メンテナンス（X軸）

      if ( sw1 == 0 )
      {
        hidari();
      }
      else if ( sw2 == 0 )
      {
        migi();
      }
      else
      {
        tomeru();
      }
      if ( sw3 == 0 && sw3f == 0 )
      {
        sw3f = 1;
        mode = 101;
        kaihei();
        kaihei();
      }
      else if ( sw3 == 1 && sw3f == 1 )
      {
        sw3f = 0;
      }
      break;
    case 101: //メンテナンス（Y軸）

      if ( sw1 == 0 )
      {
        oku();
      }
      else if ( sw2 == 0 )
      {
        temae();
      }
      else
      {
        tomeru();
      }
      if ( sw3 == 0 && sw3f == 0 )
      {
        sw3f = 1;
        mode = 102;
        kaihei();
        kaihei();
        kaihei();
      }
      else if ( sw3 == 1 && sw3f == 1 )
      {
        sw3f = 0;
      }
      break;
    case 102: //メンテナンス（Z軸）

      if ( sw1 == 0 )
      {
        orosu();
      }
      else if ( sw2 == 0 )
      {
        ageru();
      }
      else
      {
        tomeru();
      }
      if ( sw3 == 0 && sw3f == 0 )
      {
        sw3f = 1;
        mode = 1;
        tc = 0;
      }
      else if ( sw3 == 1 && sw3f == 1 )
      {
        sw3f = 0;
      }
      break;
  }

  //プログラムを1msごとに動かすためのdelay
  delay(1);
}
void migi () //アームを右側に移動
{
  PORTL = 0x40;
}

void hidari () //アームを左側に移動
{
  PORTL = 0x80;
}

void oku () //アームを奥側に移動
{
  PORTL = 0x10;
}

void temae () //アームを手前側に移動
{
  PORTL = 0x20;
}

void modosu () //アームを斜め移動
{
  PORTL = 0xa0;
}

void orosu () //アームを下げる
{
  PORTL = 0x08;
}

void ageru () //アームをあげる
{
  PORTL = 0x04;
}

void tomeru () //モーターをすべて止める
{
  PORTL = 0x00;
}

void toziru () //アームを閉じる
{
  digitalWrite(12, HIGH);
}

void hiraku() //アームを開く
{
  digitalWrite(12, LOW);
}

void kaihei () //アームの開閉
{
  toziru();
  delay(500);
  hiraku();
  delay(500);
}
