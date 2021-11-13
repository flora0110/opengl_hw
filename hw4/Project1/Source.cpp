﻿/***************************
4108056029 王傳馨 第IV次作業11/10
***************************/

#include <GL/glut.h>
//#include <stdlib.h>
#include <iostream>
#include <sstream>
using namespace std;

#define WIDTH 1920.0f
#define HEIGHT 1080.0f
class NCHU {
protected:
    float startx, starty, middlex, middley;
    int check(int(*data)[2], int len) {
	   int cross;
	   int Ax, Ay;
	   int Bx, By;
	   for (int i = 2; i < len; i++) {
		  Ax = data[i - 1][0] - data[i - 2][0];
		  Ay = data[i - 1][1] - data[i - 2][1];
		  Bx = data[i][0] - data[i - 1][0];
		  By = data[i][1] - data[i - 1][1];
		  cross = Ax * By - Ay * Bx;
		  if (cross > 0) {
			 return 1;
		  }
		  else if (cross < 0) {
			 return -1;
		  }
	   }
	   return 0;
    }
    void draw_ch(int(*data)[2], int len) {
	   int cross = check(data, len);
	   if (cross == 1) {
		  glBegin(GL_POLYGON);
		  for (int i = 0; i < len; i++) {
			 //glVertex2f(((data[i][0] - middlex) / startx), ((middley - data[i][1]) / starty));
			 glVertex3f(((data[i][0] - middlex) / startx), ((middley - data[i][1]) / starty), 0.0);
		  }
		  glEnd();
	   }
	   else if (cross == -1) {
		  int reverse_data[20][2];
		  int j = 0;
		  for (int i = len - 1; i >= 0; i--) {
			 reverse_data[j][0] = data[i][0];
			 reverse_data[j++][1] = data[i][1];
		  }
		  glBegin(GL_POLYGON);
		  for (int i = 0; i < len; i++) {
			 //glVertex2f(((data[i][0] - middlex) / startx), ((middley - data[i][1]) / starty));
			 glVertex3f(((data[i][0] - middlex) / startx), ((middley - data[i][1]) / starty), 0.0);
		  }
		  glEnd();
	   }
    }

public:
    NCHU(float width, float height) {
	   middlex = (width / 4) + 200;
	   middley = (height * 3 / 4) - 100;
	   startx = (width / 2);
	   starty = height / 2;
    }
};
class A :protected NCHU {
private:
    //A
    int a_5[8][2] = { {73, 629},{86, 611},{120,607},{139,605},{171,606}, {170,617},{138,618},{119,619}, };
    int a_4[7][2] = { {94,601},{142,579},{143,567},{125,572},{103,580},{89,589},{87,597} , };
    int a_3[8][2] = { {129,541},{144,537},{153,554},{161,576},{173,591},{169,602},{155,587} ,{148,577} };
    int a_three[2][3][2] = {
	   {{178,621},{178,645},{168,622},},
	   {{119,558},{125,572},{103,580}, },
    };
    int a_four[10][4][2] = {
	   {{73, 519},{92, 532},{86, 611},{73, 629},},
	   {{92,532},{83,566},{98,555},{99,542}},
	   {{98,555},{99,542},{128,534},{129,541}},
	   {{128,534},{129,541},{144,537},{140,529}},
	   {{144,537},{140,529},{149,527},{166,535}},
	   {{128,534},{140,529},{140,518},{127,522}},
	   {{112,559},{119,558,},{103,580},{99,575}},
	   {{119,558},{135,542},{139,554},{125,572}},
	   {{158,541},{167,553},{161,576},{153,554}},
	   {{148,577},{155,587},{138,605},{115,606}},
    };
    int a_five[4][5][2] = {
	   {{161,499},{180,498},{176,511},{92, 532},{73, 519},},
	   {{180,498},{176,511},{178,621},{178,645},{189,628},},
	   {{149,527},{166,535},{166,529},{158,512},{146,516}},
	   {{84,584},{109,550},{129,540},{112,559},{99,575}},
    };

public:
    A(float width, float height) :NCHU(width, height) {
    }
    void display_ch() {
	   glColor3f(0.007, 0.4375, 0.589);
	   draw_ch(a_3, sizeof(a_3) / sizeof(a_3[0]));
	   draw_ch(a_4, sizeof(a_4) / sizeof(a_4[0]));
	   draw_ch(a_5, sizeof(a_5) / sizeof(a_5[0]));
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_three[i], sizeof(a_three[i]) / sizeof(a_three[i][0]));
	   }
	   for (int i = 0; i < 10; i++) {
		  draw_ch(a_four[i], sizeof(a_four[i]) / sizeof(a_four[i][0]));
	   }
	   for (int i = 0; i < 4; i++) {
		  draw_ch(a_five[i], sizeof(a_five[i]) / sizeof(a_five[i][0]));
	   }
    }
};
class B :protected NCHU {
private:
    int a_four[2][4][2] = {
	   {{269,588},{287,565},{314,553},{313,559}},
	   {{314,553},{313,559},{318,560},{327,554}},
    };
    int a_five[2][5][2] = {
	   {{275,520},{275,508},{308,522},{308,529},{287,533}},
	   {{318,560},{327,554},{331,561},{295,604},{277,605}},
    };
    int a_six[1][6][2] = {
	   {{292,546},{315,541},{287,565},{269,588},{260,586},{260,573}},
    };
    int a_seven[2][7][2] = {
	   {{308,529},{287,533},{244,542},{240,558}, {247,562},{329,541},{324,530}},
	   {{295,604},{277,605},{238,608},{249,622},{314,613},{352,612},{346,602}},
    };

public:
    B(float width, float height) :NCHU(width, height) {
    }
    void display_ch() {
	   glColor3f(0.007, 0.4375, 0.589);
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_four[i], sizeof(a_four[i]) / sizeof(a_four[i][0]));
	   }
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_five[i], sizeof(a_five[i]) / sizeof(a_five[i][0]));
	   }
	   for (int i = 0; i < 1; i++) {
		  draw_ch(a_six[i], sizeof(a_six[i]) / sizeof(a_six[i][0]));
	   }
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_seven[i], sizeof(a_seven[i]) / sizeof(a_seven[i][0]));
	   }

    }
};
class C :protected NCHU {
private:
    int a_four[4][4][2] = {
	   {{442,501},{452,499},{450,671},{438,680}},
	   {{377,529},{388,535},{396,533},{393,529}},
	   {{439,527} ,{436,519},{480,517},{471,524}},
	   {{480,517},{471,524},{489,529},{489,522}},
    };
    int a_five[1][5][2] = {
	   {{471,524},{489,529},{447,563},{414,575},{415,536}},
    };
    int a_six[2][6][2] = {
	   {{471,524},{489,529},{447,563},{414,575},{415,566},{447,551}},
	   {{439,527} ,{436,519} ,{395,525},{393,529},{396,533},{406,542}},
    };
    int a_seven[2][7][2] = {
	   {{377,529},{388,535},{391,535},{398,580},{389,578},{378,561},{374,534}},
	   {{414,575},{415,566},{447,557},{460,556},{469,558},{465,568},{444,575}},
    };

public:
    C(float width, float height) :NCHU(width, height) {
    }
    void display_ch() {
	   glColor3f(0.007, 0.4375, 0.589);
	   for (int i = 0; i < 4; i++) {
		  draw_ch(a_four[i], sizeof(a_four[i]) / sizeof(a_four[i][0]));
	   }
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_six[i], sizeof(a_six[i]) / sizeof(a_six[i][0]));
	   }
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_seven[i], sizeof(a_seven[i]) / sizeof(a_seven[i][0]));
	   }
    }
};
class D :protected NCHU {
private:
    int a_three[2][3][2] = {
	   {{608,547},{615,562},{613,565}},
	   {{577,604},{581,598},{597,595}},
    };
    int a_four[13][4][2] = {
	   {{540,516},{551,513},{554,520},{544,522}},
	   {{554,520},{544,522},{544,582},{553,579}},
	   {{554,512},{562,510},{569,518},{560,518}},
	   {{572,511},{599,499},{608,498},{612,504}},
	   {{596,536},{584,538},{586,541},{591,545}},
	   {{598,549},{598,553},{587,555},{587,550}},
	   {{605,517},{606,509},{621,507},{623,517}},
	   {{623,517},{631,519},{626,538},{620,534}},
	   {{601,543},{615,543},{618,548},{601,546}},
	   {{611,558},{614,556},{622,565},{614,568}},
	   {{651,556},{657,555},{660,570},{651,565} },
	   {{526,603},{529,619},{541,627},{541,609}},
	   {{654,632},{659,623},{659,612},{623,589}},
    };
    int a_five[6][5][2] = {
	   {{560,518},{569,518},{569,574},{563,585},{559,574}},
	   {{599,499},{608,498},{612,504},{601,578},{597,580}},
	   {{577,511},{583,510},{583,572},{577,580},{576,572}},
	   {{586,541},{591,545},{588,565},{581,564},{581,544}},
	   {{621,507},{623,517},{631,519},{637,501},{628,501}},
	   {{541,627},{541,609},{555,604},{574,601},{571,610}},
    };
    int a_six[2][6][2] = {
	   {{626,538},{620,534},{617,559},{613,568},{615,576},{624,565}},
	   {{570,579},{584,568},{620,562},{660,559},{660,570},{662,570}},
    };
    int a_seven[2][7][2] = {
	   {{574,541},{574,533},{588,524},{596,522},{608,527},{596,536},{584,538}},
	   {{626,538},{620,534},{607,527},{602,530},{622,552},{627,551},{631,542}},
    };
    int a_nine[9][2] = { {547,577},{506,586},{495,586},{493,601},{499,607},{504,607},{523,595},{570,579},{584,568} };

public:
    D(float width, float height) :NCHU(width, height) {
    }
    void display_ch() {
	   glColor3f(0.007, 0.4375, 0.589);
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_three[i], sizeof(a_three[i]) / sizeof(a_three[i][0]));
	   }
	   for (int i = 0; i < 13; i++) {
		  draw_ch(a_four[i], sizeof(a_four[i]) / sizeof(a_four[i][0]));
	   }
	   for (int i = 0; i < 6; i++) {
		  draw_ch(a_five[i], sizeof(a_five[i]) / sizeof(a_five[i][0]));
	   }
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_six[i], sizeof(a_six[i]) / sizeof(a_six[i][0]));
	   }
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_seven[i], sizeof(a_seven[i]) / sizeof(a_seven[i][0]));
	   }
	   draw_ch(a_nine, sizeof(a_nine) / sizeof(a_nine[0]));
    }
};
class E :protected NCHU {
private:
    int a_three[1][3][2] = {
	   {{769,503},{769,525},{762,525}},
    };
    int a_four[10][4][2] = {
	   {{723,553},{729,564},{760,551},{759,540}},
	   {{760,551},{759,540},{768,538},{768,549}},
	   {{768,549},{768,538},{785,535},{788,548}},
	   {{766,507},{771,508},{770,539},{768,540}},
	   {{762,525},{767,522},{764,542},{757,544}},
	   {{755,546},{761,549},{759,565},{751,563}},
	   {{759,565},{751,563},{744,577},{755,577}},
	   {{722,599},{722,602},{712,605},{716,600}},
	   {{770,577},{799,586},{809,593},{789,593}},
	   {{789,593},{790,604},{774,604},{782,599}},
    };
    int a_five[1][5][2] = {
	   {{750,590},{736,586},{721,596},{720,604},{735,604}},
    };
    int a_six[2][6][2] = {
	   {{744,577},{755,577},{750,589},{744,597},{750,590},{736,586}},
	   {{809,593},{789,593},{790,604},{801,611},{805,610},{809,606}},
    };
    int a_seven[2][7][2] = {
    };
    int a_eight[8][2] = { {728,552},{729,564} ,{725,570},{717,574},{706,575},{700,568},{702,557},{713,553} };

public:
    E(float width, float height) :NCHU(width, height) {
    }
    void display_ch() {
	   glColor3f(0.007, 0.4375, 0.589);
	   for (int i = 0; i < 1; i++) {
		  draw_ch(a_three[i], sizeof(a_three[i]) / sizeof(a_three[i][0]));
	   }
	   for (int i = 0; i < 10; i++) {
		  draw_ch(a_four[i], sizeof(a_four[i]) / sizeof(a_four[i][0]));
	   }
	   for (int i = 0; i < 1; i++) {
		  draw_ch(a_five[i], sizeof(a_five[i]) / sizeof(a_five[i][0]));
	   }
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_six[i], sizeof(a_six[i]) / sizeof(a_six[i][0]));
	   }
	   draw_ch(a_eight, sizeof(a_eight) / sizeof(a_eight[0]));
    }
};
class F :protected NCHU {
private:
    int a_three[7][3][2] = {
	   {{871,521},{874,515},{865,513}},
	   {{909,513},{911,507},{904,508}},
	   {{955,563},{940,565},{940,556}},
	   {{940,565},{940,556},{924,571}},
	   {{914,598},{901,598},{905,608}},
	   {{911,644},{896,638},{904,623}},
	   {{843,674},{848,669},{826,665}},
    };
    int a_four[22][4][2] = {
	   {{860,507},{869,493},{875,502},{867,512}},
	   {{876,540},{869,537},{883,525},{890,526}},
	   {{854,554},{863,554},{863,567},{854,570}},
	   { {887, 548}, { 891,539 }, { 882,533 }, {877,537}},
	   {{880,558},{895,554},{900,545},{890,535}},
	   {{900,545},{890,535},{898,525},{901,529}},
	   {{898,525}, {883,525},{876,516},{907,513}},
	   {{885,516},{896,513},{904,504},{896,515}},
	   {{909,513},{911,507},{920,503},{921,513}},
	   {{925,531},{922,517},{921,513},{931,512}},
	   {{910,544},{926,541},{921,550},{908,553}},
	   {{840,593},{846,585},{843,568},{826,590}},
	   {{846,585},{857,576},{859,564},{841,576}},
	   {{887,565},{935,553},{942,545},{895,554}},
	   {{899,583},{896,572},{884,577},{894,583}},
	   {{914,589},{913,598},{923,595},{934,588}},
	   {{923,595},{934,588},{940,594},{937,598}},
	   {{901,598},{895,592},{872,599},{876,604}},
	   {{911,644},{914,598},{905,608},{904,623}},
	   {{911,644},{896,638},{873,658},{893,660}},
	   {{873,658},{893,660},{871,672},{848,669}},
	   {{873,658},{871,672},{843,674},{848,669}},
    };
    int a_five[4][5][2] = {
	   {{909,521},{907,530},{902,525},{901,522},{904,515}},
	   {{832,607},{840,602},{840,593},{826,590},{826,599}},
	   {{857,576},{887,565},{895,554},{880,557},{859,565}},
	   {{872,599},{876,604},{871,605},{867,603},{867,598}},
    };
    int a_six[2][6][2] = {
	   {{921,513},{931,512},{935,506},{935,499},{930,497},{920,503}},
	   {{910,544},{926,541},{925,531},{922,517},{909,521},{907,530}},
    };
    int a_seven[5][7][2] = {
	   {{860,507},{867,512},{873,524},{865,536},{853,536},{848,524},{852,504}},
	   {{876,540},{869,537},{866,536},{853,536},{854,554},{863,554},{875,544}},
	   {{935,553},{942,545},{950,545},{958,551},{959,557},{955,563},{940,556}},
	   {{899,583},{907,583},{910,580},{912,573},{911,564},{905,564},{896,572}},
	   {{899,583},{907,583},{914,589},{913,598},{901,598},{895,592},{893,588}},
    };
    int a_eight[8][2] = { };

public:
    F(float width, float height) :NCHU(width, height) {
    }
    void display_ch() {
	   glColor3f(0.007, 0.4375, 0.589);
	   for (int i = 0; i < 7; i++) {
		  draw_ch(a_three[i], sizeof(a_three[i]) / sizeof(a_three[i][0]));
	   }
	   for (int i = 0; i < 22; i++) {
		  draw_ch(a_four[i], sizeof(a_four[i]) / sizeof(a_four[i][0]));
	   }
	   for (int i = 0; i < 4; i++) {
		  draw_ch(a_five[i], sizeof(a_five[i]) / sizeof(a_five[i][0]));
	   }
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_six[i], sizeof(a_six[i]) / sizeof(a_six[i][0]));
	   }
	   for (int i = 0; i < 5; i++) {
		  draw_ch(a_seven[i], sizeof(a_seven[i]) / sizeof(a_seven[i][0]));
	   }
    }
};
static GLfloat color[3] = { 0.007, 0.4375, 0.589 };
class Logo :protected NCHU {
private:
    int a_three[5][3][2] = {
	   {{1198,517},{1212,482},{1159,490}},
	   //left
	   {{1175,633},{1162,630},{1169,615}},

	   {{1034,649},{1058,638},{1044,622}},
	   {{1074,509},{1056,515},{1053,499}},

	   {{1127,586},{1057,569},{1107,538}},
    };
    int a_four[18][4][2] = {
	   {{1224,652},{1206,607},{1175,613},{1179,634}},
	   {{1206,607},{1175,613},{1162,581},{1199,575}},
	   {{1162,581},{1199,575},{1197,550},{1148,541}},
	   {{1197,550},{1148,541},{1117,523},{1198,517}},

	   {{1117,523},{1198,517},{1212,482},{1159,490}},
	   {{1159,490},{1116,490},{1101,509},{1117,523}},
	   {{1101,509},{1085,500},{1092,488},{1116,490}},
	   {{1117,523},{1159,490},{1116,490},{1148,541}},

	   {{1085,500},{1092,488},{1069,483},{1066,492}},
	   {{1069,483},{1066,492},{1049,487},{1045,476}},
	   //left
	   {{1162,630},{1169,615},{1163,601},{1141,625}},

	   {{1058,638},{1044,622},{1051,597},{1076,632}},
	   {{1051,597},{1076,632},{1099,627},{1057,569} },
	   {{1099,627},{1057,569},{1127,587},{1122,625} },

	   {{1057,569},{1129,586},{1105,528},{1059,545}},
	   {{1105,528},{1059,545},{1058,531},{1093,519}},
	   {{1058,531},{1093,519},{1074,509},{1056,515}},
	   {{1127,586},{1105,528},{1066,528},{1064,578}},

    };
    int a_five[1][5][2] = {
	   //left
	   {{1163,601},{1141,625},{1122,625},{1127,587},{1156,589}},
    };
    //red
    int red_three[1][3][2] = {
	   {{1161,579},{1166,569},{1156,570}},
    };
    int red_four[5][4][2] = {
	   {{1166,569},{1156,570},{1150,561},{1168,557}},
	   {{1150,561},{1168,557},{1164,543},{1142,550}},
	   {{1164,543},{1142,550},{1134,541},{1156,531}},
	   {{1134,541},{1156,531},{1143,525},{1125,532}},
	   {{1143,525},{1125,532},{1117,523},{1126,521}},
    };
    int ball[15][2] = { {1155,589},{1150,578},{1142,566},{1133,554},{1124,545},{1114,536},{1105,528},
	   {1098,537},{1092,546},{1093,571},{1098,581},{1107,590},{1118,595},{1131,596},{1142,594} };

    int ball_2[15][2] = { {1141,576},{1132,580},{1121,581},{1112,580},{1102,576},{1093,571},{1089,558},{1092,546},{1097,537},
	 {1105,528},{1114,536},{1124,545},{1133,554}, {1142,566},{1146,572},
    };
    int r_ball_2[2][6][2] = {
	   {{1152,563},{1156,552},{1156,543},{1154,533},{1144,527},{1142,550}},
	   {{1144,527},{1142,550},{1134,540},{1125,530},{1118,525},{1132,523}},
    };

    int ball_3[9][2] = {
	   {1106,529}, {1114,536},{1124,545},{1133,554},{1136,559},{1125,561},{1114,560},{1105,552},{1101,541}
    };
    int r_ball_3[7][2] = {
	   {1142,550},{1134,540},{1125,530},{1118,525},{1132,523},{1142,530},{1144,540},
    };

public:

    Logo(float width, float height) :NCHU(width, height) {
    }
    void display_ch() {
	   //cout << "dis" << endl;
	   //cout << color[1] << endl;
	   //glColor3f(0.007, 0.4375, 0.589);
	   glColor3f(color[0], color[1], color[2]);
	   for (int i = 0; i < 5; i++) {
		  draw_ch(a_three[i], sizeof(a_three[i]) / sizeof(a_three[i][0]));
	   }
	   for (int i = 0; i < 18; i++) {
		  draw_ch(a_four[i], sizeof(a_four[i]) / sizeof(a_four[i][0]));
	   }
	   for (int i = 0; i < 1; i++) {
		  draw_ch(a_five[i], sizeof(a_five[i]) / sizeof(a_five[i][0]));
	   }/*
	   for (int i = 0; i < 2; i++) {
		  draw_ch(a_six[i], sizeof(a_six[i]) / sizeof(a_six[i][0]));
	   }
	   draw_ch(a_eight, sizeof(a_eight) / sizeof(a_eight[0]));*/
	   glColor3f(0.6, 0.0, 0.0);
	   for (int i = 0; i < 1; i++) {
		  draw_ch(red_three[i], sizeof(red_three[i]) / sizeof(red_three[i][0]));
	   }
	   for (int i = 0; i < 5; i++) {
		  draw_ch(red_four[i], sizeof(red_four[i]) / sizeof(red_four[i][0]));
	   }
	   draw_ch(ball, sizeof(ball) / sizeof(ball[0]));
	   glColor3f(0.8, 0.0, 0.0);
	   draw_ch(ball_2, sizeof(ball_2) / sizeof(ball_2[0]));
	   for (int i = 0; i < 2; i++) {
		  draw_ch(r_ball_2[i], sizeof(r_ball_2[i]) / sizeof(r_ball_2[i][0]));
	   }
	   glColor3f(1.0, 0.0, 0.0);
	   draw_ch(ball_3, sizeof(ball_3) / sizeof(ball_3[0]));
	   draw_ch(r_ball_3, sizeof(r_ball_3) / sizeof(r_ball_3[0]));
    }
};
class NCHU_en {
protected:
    float startx, starty, s, middlex, middley;

    int check(int(*data)[2], int len) {
	   int cross;
	   int Ax, Ay;
	   int Bx, By;
	   for (int i = 2; i < len; i++) {
		  Ax = data[i - 1][0] - data[i - 2][0];
		  Ay = data[i - 1][1] - data[i - 2][1];
		  Bx = data[i][0] - data[i - 1][0];
		  By = data[i][1] - data[i - 1][1];
		  cross = Ax * By - Ay * Bx;
		  if (cross > 0) {
			 return 1;
		  }
		  else if (cross < 0) {
			 return -1;
		  }
	   }
	   return 0;
    }
    void draw_en(int(*data)[2], int move) {
	   int cross = check(data, 3);
	   if (cross == 1) {
		  glBegin(GL_TRIANGLES);
		  for (int i = 0; i < 3; i++) {
			 glVertex2f(((data[i][0] - s + move) / startx), ((middley - data[i][1]) / starty));
		  }
		  glEnd();
	   }
	   else if (cross == -1) {
		  int reverse_data[20][2];
		  int j = 0;
		  for (int i = 2; i >= 0; i--) {
			 reverse_data[j][0] = data[i][0];
			 reverse_data[j++][1] = data[i][1];
		  }
		  glBegin(GL_TRIANGLES);
		  for (int i = 0; i < 3; i++) {
			 glVertex2f(((data[i][0] - s + move) / startx), ((middley - data[i][1]) / starty));
		  }
		  glEnd();
	   }
    }
public:
    NCHU_en(float width, float height) {
	   startx = width / 2;
	   starty = height / 2;
	   middlex = width / 4 + 200;
	   middley = (height * 3 / 4) - 100;
	   s = middlex + 40.0;
    }
};
class National :protected NCHU_en {
private:
    int N[7][3][2] = {
	   {{123,790},{129,790},{123,743}},
	   {{129,790},{123,743},{129,756}},

	   {{123,743},{129,756},{129,743}},

	   {{129,756},{129,743},{155,790}},
	   {{129,743},{155,790},{163,790}},
	   {{155,790},{163,790},{163,743}},
	   {{163,743},{157,743},{157,790}},
    };
    int a[26][3][2] = {
	   {{207,791},{206,763},{201,791}},
	   {{206,763},{201,791},{200,770}},
	   {{201,787},{201,779},{191,792}},
	   {{201,779},{191,792},{189,788}},
	   {{191,792},{189,788},{181,791}},
	   {{189,788},{181,791},{183,786}},
	   {{181,791},{183,786},{175,784}},
	   {{183,786},{175,784},{181,780}},
	   {{175,784},{181,780},{179,774}},
	   {{181,780},{179,774},{185,776}},
	   {{179,774},{185,776},{191,771}},
	   {{185,776},{191,771},{200,774}},
	   {{191,771},{200,774},{200,770}},

	   {{200,770},{200,764},{206,763}},
	   {{200,764},{206,763},{202,758}},
	   {{206,763},{202,758},{196,760}},
	   {{200,764},{196,760},{202,758}},
	   {{202,758},{196,760},{196,756}},
	   {{196,760},{196,756},{191,760}},
	   {{196,756},{191,760},{188,756}},
	   {{191,760},{188,756},{187,760}},
	   {{188,756},{187,760},{181,758}},
	   {{187,760},{181,758},{183,763}},
	   {{181,758},{183,763},{177,762}},
	   {{183,763},{177,762},{176,765}},
	   {{183,763},{176,765},{182,766}},

    };
    int t[7][3][2] = {
	   {{222,787},{228,786},{228,745}},
	   {{222,787},{228,745},{222,748}},
	   {{217,761},{233,755},{233,761}},
	   {{217,761},{233,755},{217,755}},
	   {{222,787},{228,786},{226,791}},
	   {{228,786},{233,791},{226,791}},
	   {{228,786},{233,791},{233,787}},
    };
    int i_n_u_r[2][3][2] = {
	   {{244,790},{250,757},{250,790}},
	   {{244,790},{250,757},{244,757}},
    };
    int I[2][3][2] = {
	   {{244,749},{244,744},{250,744}},
	   {{244,749},{250,749},{250,744}},
    };
    int o_e[19][3][2] = {
	   {{278,792},{283,786},{290,786}},
	   {{283,786},{290,786},{288,781}},
	   {{290,786},{288,781},{294,781}},

	   //{{290,786},{288,781},{295,779}},
	   //{{288,781},{295,779},{289,773}},
	   //{{295,779},{289,773},{294,766}},

	   {{294,766},{289,773},{295,773}},
	   {{289,773},{294,766},{288,766}},
	   {{294,766},{288,766},{290,759}},
	   {{288,766},{290,759},{283,762}},
	   {{290,759},{283,762},{280,756}},
	   {{283,762},{280,756},{276,761}},
	   {{280,756},{276,761},{268,759}},
	   {{276,761},{268,759},{269,766}},
	   {{268,759},{269,766},{262,766}},
	   {{269,766},{262,766},{268,774}},
	   {{262,766},{268,774},{262,779}},
	   {{268,774},{262,779},{269,782}},
	   {{262,779},{269,782},{266,787}},
	   {{269,782},{266,787},{275,787}},
	   {{266,787},{275,787},{278,792}},
	   {{275,787},{278,792},{283,786}},
    };
    int o[2][3][2] = {
	   {{288,781},{295,773},{294,781}},
	   {{288,781},{295,773},{289,773}},
    };
    int n[8][3][2] = {
	   {{331,790},{336,762},{336,790}},
	   {{331,790},{336,762},{331,765}},

	   {{331,765},{331,756},{336,762}},
	   {{331,765},{331,756},{329,762}},

	   {{329,762},{320,756},{331,756}},
	   {{329,762},{320,756},{321,760}},

	   {{312,766},{320,756},{321,760}},
	   {{312,766},{320,756},{311,762}},
    };
    int L[2][3][2] = {
	   {{394,790},{399,744},{394,744}},
	   {{394,790},{399,744},{399,790}},
    };

public:
    National(float width, float height) :NCHU_en(width, height) {
    }
    void display_en() {
	   //float alldata[LEN];
	   //GLint first[PRICOUNT];
	   //GLsizei count[PRICOUNT];
	   int k = 0, primcount = 0;
	   glColor3f(0.007, 0.4375, 0.589);
	   //first[primcount] = 0;
	   for (int i = 0; i < 7; i++) {

		  draw_en(N[i], 0);
	   }
	   for (int i = 0; i < 26; i++) {

		  draw_en(a[i], 0);
		  draw_en(a[i], 173);
	   }
	   for (int i = 0; i < 7; i++) {
		  draw_en(t[i], 0);
		  draw_en(t[i], 957);
	   }
	   for (int i = 0; i < 2; i++) {
		  draw_en(i_n_u_r[i], 0);//i
		  draw_en(I[i], 0);
		  draw_en(i_n_u_r[i], 63);//n
		  draw_en(i_n_u_r[i], 306);//u
		  draw_en(i_n_u_r[i], 317 + 8);//n
		  draw_en(i_n_u_r[i], 393);//g
		  draw_en(i_n_u_r[i], 523);//i
		  draw_en(I[i], 523);
		  draw_en(i_n_u_r[i], 543);//n
		  draw_en(i_n_u_r[i], 611);//g
		  draw_en(i_n_u_r[i], 701);//n
		  draw_en(i_n_u_r[i], 744);//i
		  draw_en(I[i], 744);
		  draw_en(i_n_u_r[i], 846);//r
		  draw_en(i_n_u_r[i], 913);//i
		  draw_en(I[i], 913);
		  draw_en(L[i], 0);
		  draw_en(L[i], 91);
		  draw_en(L[i], 281);
		  draw_en(L[i], 315);
		  draw_en(o[i], 0);
	   }
	   for (int i = 0; i < 19; i++) {
		  draw_en(o_e[i], 0);
		  draw_en(o_e[i], 783);
	   }
	   for (int i = 0; i < 8; i++) {
		  draw_en(n[i], 0);
		  //draw_en(n[i], 63-63);//n
		  draw_en(n[i], 317 - 63 + 8);//n
		  draw_en(n[i], 543 - 63);//n
		  draw_en(n[i], 701 - 63);//n
		  draw_en(n[i], 177);//n
	   }
    }
};
class Chung :protected NCHU_en {
private:
    int C[21][3][2] = {
	   {{472,776},{465,776},{467,785}},
	   {{465,776},{467,785},{460,783}},
	   {{467,785},{460,783},{457,791}},
	   {{460,783},{457,791},{451,786}},
	   {{457,791},{451,786},{445,792}},
	   {{451,786},{445,792},{441,783}},
	   {{445,792},{441,783},{433,785}},
	   {{441,783},{433,785},{435,775}},
	   {{433,785},{435,775},{428,773}},
	   {{435,775},{428,773},{434,766}},
	   {{428,773},{434,766},{429,758}},
	   {{434,766},{429,758},{436,757}},
	   {{429,758},{436,757},{434,747}},
	   {{434,766},{434,747},{441,750}},
	   {{434,747},{441,750},{445,743}},
	   {{441,750},{445,743},{451,749}},
	   {{445,743},{451,749},{458,742}},
	   {{451,749},{458,742},{460,750}},
	   {{458,742},{460,750},{467,748}},
	   {{460,750},{467,748},{465,757}},
	   {{467,748},{465,757},{470,756}},
    };
    int u[10][3][2] = {
	   {{527,783},{533,756},{527,756}},
	   {{527,783},{533,756},{533,781}},

	   {{527,783},{536,786},{533,781}},
	   {{527,783},{536,786},{530,788}},
	   //{{533,781},{536,786},{530,788}},

	   {{536,786},{530,788},{536,792}},
	   {{530,788},{536,792},{541,787}},
	   {{536,792},{541,787},{545,791}},
	   {{541,787},{545,791},{547,785}},
	   {{545,791},{547,785},{552,784}},
	   {{547,785},{552,784},{553,778}},
    };
    int g[28][3][2] = {
	   {{613,796},{618,795},{614,800}},
	   {{618,795},{614,800},{620,799}},
	   {{614,800},{620,799},{621,805}},
	   {{620,799},{621,805},{626,801}},
	   {{621,805},{626,801},{630,806}},
	   {{626,801},{630,806},{632,800}},
	   {{630,806},{632,800},{639,801}},
	   {{632,800},{639,801},{637,795}},
	   {{639,801},{637,795},{643,795}},
	   {{643,795},{637,787},{637,795}},
	   {{643,795},{637,787},{644,787}},

	   {{637,780},{637,787},{632,786}},
	   {{637,787},{632,786},{631,791}},
	   {{632,786},{631,791},{627,787}},
	   {{631,791},{627,787},{622,790}},
	   {{627,787},{622,790},{622,785}},

	   {{622,790},{622,785},{615,785}},
	   {{622,785},{615,785},{618,781}},
	   {{615,785},{618,781},{612,780}},

	   {{618,781},{612,780},{618,767}},

	   {{612,780},{618,767},{612,769}},
	   {{618,767},{612,769},{615,761}},

	   {{618,767},{615,761},{622,762}},
	   {{615,761},{622,762},{622,756}},
	   {{622,762},{622,756},{628,760}},
	   {{622,756},{628,760},{633,757}},
	   {{628,760},{633,757},{636,764}},
	   {{633,757},{636,764},{639,764}},
    };
public:
    Chung(float width, float height) :NCHU_en(width, height) {
    }
    void display_en() {
	   glColor3f(0.007, 0.4375, 0.589);
	   for (int i = 0; i < 21; i++) {
		  draw_en(C[i], 0);
	   }
	   for (int i = 0; i < 10; i++) {
		  draw_en(u[i], 0);
	   }
	   for (int i = 0; i < 28; i++) {
		  draw_en(g[i], 0);
		  draw_en(g[i], 218);
	   }
    }
};
class Hsing :protected NCHU_en {
private:
    int H[2][3][2] = {
	   {{710,768},{678,763},{710,763}},
	   {{710,768},{678,763},{678,768}},
    };
    int s[26][3][2] = {
	   {{726,781},{731,781},{729,787}},
	   {{731,781},{729,787},{734,786}},
	   {{729,787},{734,786},{735,791}},

	   {{734,786},{735,791},{739,788}},
	   {{735,791},{739,788},{742,792}},
	   {{739,788},{742,792},{745,788}},

	   {{742,792},{745,788},{751,790}},
	   {{745,788},{751,790},{750,784}},
	   {{751,790},{750,784},{756,784}},

	   {{750,784},{755,778},{756,784}},
	   {{750,784},{755,778},{750,780}},

	   {{750,780},{750,774},{755,778}},
	   {{750,780},{750,774},{745,778}},

	   {{737,775},{750,774},{745,778}},
	   {{737,775},{750,774},{737,769}},

	   {{737,775},{737,769},{730,772}},
	   {{737,769},{730,772},{733,766}},
	   {{730,772},{733,766} ,{726,767}},
	   {{726,767},{735,761},{733,766}},
	   {{726,767},{735,761},{728,760}},
	   {{735,761},{728,760},{736,756}},

	   {{736,756},{746,761},{735,760}},
	   {{736,756},{746,761},{745,755}},

	   {{746,761},{745,755},{752,760}},

	   {{746,761},{754,764},{752,760}},
	   {{746,761},{754,764},{749,766}},

    };

public:
    Hsing(float width, float height) :NCHU_en(width, height) {
    }
    void display_en() {
	   glColor3f(0.007, 0.4375, 0.589);
	   for (int i = 0; i < 2; i++) {
		  draw_en(H[i], 0);
	   }
	   for (int i = 0; i < 26; i++) {
		  draw_en(s[i], 0);
		  draw_en(s[i], 389);
	   }
    }
};
class University :protected NCHU_en {
private:
    int U[13][3][2] = {
	   {{891,745},{897,780},{891,780}},
	   {{891,745},{897,780},{897,745}},
	   {{891,780},{897,780},{896,787}},
	   {{897,780},{896,787},{903,785}},
	   {{896,787},{903,785},{904,792}},
	   {{903,785},{904,792},{911,787}},
	   {{904,792},{911,787},{916,792}},
	   {{911,787},{916,792},{918,785}},
	   {{916,792},{918,785},{926,787}},
	   {{918,785},{926,787},{923,779}},
	   {{926,787},{923,779},{930,780}},
	   {{930,780},{923,745},{930,745}},
	   {{930,780},{923,745},{923,780}},
    };
    int v[4][3][2] = {
	   {{1024,791},{1004,757},{1018,791}},
	   {{1024,791},{1004,757},{1010,757}},
	   {{1018,791},{1038,757},{1031,757}},
	   {{1018,791},{1038,757},{1024,791}},
    };
    int e[2][3][2] = {
	   {{1078,774},{1046,771},{1078,771}},
	   {{1078,774},{1046,771},{1046,774}},
    };
    int r[6][3][2] = {
	   {{1110,758},{1108,762},{1106,756}},
	   {{1108,762},{1106,756},{1103,761}},
	   {{1106,756},{1103,761},{1101,756}},
	   {{1103,761},{1101,756},{1098,764}},
	   {{1101,756},{1098,764},{1095,762}},
	   {{1098,764},{1095,762},{1095,770}},

    };
    int y[6][3][2] = {
	   {{1200,800},{1205,805},{1205,800}},
	   {{1200,800},{1205,805},{1200,805}},
	   {{1205,805},{1205,800},{1210,803}},
	   {{1210,803},{1211,791},{1205,800}},
	   {{1210,803},{1211,791},{1213,799}},
	   {{1211,791},{1213,799},{1217,790}},
    };
public:
    University(float width, float height) :NCHU_en(width, height) {
    }
    void display_en() {
	   glColor3f(0.007, 0.4375, 0.589);
	   for (int i = 0; i < 13; i++) {
		  draw_en(U[i], 0);
	   }
	   for (int i = 0; i < 4; i++) {
		  draw_en(v[i], 0);
		  draw_en(v[i], 193);
	   }
	   for (int i = 0; i < 2; i++) {
		  draw_en(e[i], 0);
	   }
	   for (int i = 0; i < 6; i++) {
		  draw_en(r[i], 0);
		  draw_en(y[i], 0);
	   }
    }
};
void init(void)
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    //glClearColor(1.0, 1.0, 1.0, 0.0);
    //glShadeModel(GL_FLAT);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    //
    GLfloat ambient[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat position[] = { 0.0, 3.0, 2.0, 0.0 };
    GLfloat lmodel_ambient[] = { 0.4, 0.4, 0.4, 1.0 };
    GLfloat local_view[] = { 0.0 };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);
    //
}
static GLfloat spin = 0.0;
static GLfloat meX = 0.0, meY = 0.0, meZ = 2.0;
//static GLfloat meX =-2.0, meY = 0.0, meZ = 0.0;
static GLfloat seeX = 0.0, seeY = 0.0, seeZ = -1.0;
static int year = 0, day = 0;
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glClearColor(1.0, 1.0, 1.0, 0.0);
    //glClearColor(0.0, 0.0, 0.0, 0.0);

    //®y¼Ð
    glPushMatrix();
    gluLookAt(meX, meY, meZ, seeX, seeY, seeZ, 0.0, 1.0, 0.0);
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(0.0, 0.0);
    glVertex2f(0.0, 10.0);
    glEnd();
    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(10.0, 0.0, 0.0);
    glEnd();
    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 10.0);
    glEnd();
    glPopMatrix();

    //logo
    A a(WIDTH, HEIGHT);
    C c(WIDTH, HEIGHT);
    D d(WIDTH, HEIGHT);
    E e(WIDTH, HEIGHT);
    F f(WIDTH, HEIGHT);
    B b(WIDTH, HEIGHT);
    Logo logo(WIDTH, HEIGHT);
    National national(WIDTH, HEIGHT);
    Chung chung(WIDTH, HEIGHT);
    Hsing hsing(WIDTH, HEIGHT);
    University university(WIDTH, HEIGHT);
    //*****************************
    GLfloat no_mat[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat mat_ambient[] = { 0.7, 0.7, 0.7, 1.0 };
    GLfloat mat_ambient_color[] = { 0.8, 0.8, 0.2, 1.0 };
    GLfloat mat_diffuse[] = { 0.1, 0.5, 0.8, 1.0 };
    GLfloat mat_diffuse_test[]={51.0 / 255.0, 52.0 / 255.0, 54.0 / 255.0};
    GLfloat mat_diffuse_black[] = { 0.04,0.04,0.04};
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat no_shininess[] = { 0.0 };
    GLfloat low_shininess[] = { 5.0 };
    GLfloat high_shininess[] = { 100.0 };
    GLfloat mat_emission[] = { 0.3, 0.2, 0.2, 0.0 };
    //*****************************

    glColor3f(0.0, 0.0, 0.0);
    glPushMatrix();
    glLoadIdentity();            //clear the matrix
    gluLookAt(meX, meY, meZ, seeX, seeY, seeZ, 0.0, 1.0, 0.0);
    glPushMatrix();
	   //gluLookAt(meX, meY, meZ, seeX, seeY, seeZ, 0.0, 1.0, 0.0);
	   a.display_ch();
	   c.display_ch();
	   d.display_ch();
	   e.display_ch();
	   f.display_ch();
	   b.display_ch();
	   glPushMatrix();
		  glDisable(GL_DEPTH_TEST);
		  glTranslatef((1125 - (WIDTH / 4 + 200)) / (WIDTH / 2), 0, 0);
		  glRotatef(spin, 0.0, 1.0, 0.0);
		  glTranslatef((-1125 + (WIDTH / 4 + 200)) / (WIDTH / 2), 0, 0);
		  logo.display_ch();
		  glEnable(GL_DEPTH_TEST);
	   glPopMatrix();
	   national.display_en();
	   chung.display_en();
	   hsing.display_en();
	   university.display_en();
    glPopMatrix();

    //walk
    glTranslatef(meX, meY, meZ - 2);
    glPushMatrix();
	   glPushMatrix();
	   glEnable(GL_DEPTH_TEST);
	   glEnable(GL_LIGHTING);
	   glEnable(GL_LIGHT0);

	   glMaterialfv(GL_FRONT, GL_AMBIENT, no_mat);
	   glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_test);
	   glMaterialfv(GL_FRONT, GL_SPECULAR, no_mat);
	   glMaterialfv(GL_FRONT, GL_SHININESS, no_shininess);
	   glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);
	   glColor3f(176.0 / 255.0, 178.0 / 255.0, 181.0 / 255.0);
	   //glTranslatef(meX, meY, meZ - 2);
	   glTranslatef(1.2, 0, 0);
	   glRotatef(310, 0.0, 1.0, 0.0);
	   //arm
	   glPushMatrix();
		  glTranslatef(0.5, 0, 0.8);
		  glRotatef(30, 0.0, 0.0, 1.0);
		  glScalef(0.4, 0.15, 0.15);      // modeling transformation
		  glutSolidCube(1.0);
		  
	   glPopMatrix();
	   glPushMatrix();
		  
		  glTranslatef(0.1, 0, 0.8);
		  glRotatef(330, 0.0, 0.0, 1.0);
		  glScalef(0.4, 0.15, 0.15);      // modeling transformation
		  glutSolidCube(1.0);
		  
	   glPopMatrix();

	   //finger1
	   glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_black);
	   glPushMatrix();
		  glTranslatef(-0.06, 0.23, 0.87);
		  glRotatef(270, 0.0, 0.0, 1.0);
		  glScalef(0.12, 0.03, 0.03);
		  glutSolidCube(1.0);
	   glPopMatrix();
	   glPushMatrix();
		  glTranslatef(-0.13, 0.27, 0.87);
		  glRotatef(0, 0.0, 0.0, 1.0);
		  glScalef(0.12, 0.03, 0.03);
		  glutSolidCube(1.0);
	   glPopMatrix();
	   //finger2
	   glPushMatrix();
		  glTranslatef(-0.07, 0.23, 0.81);
		  glRotatef(300, 0.0, 0.0, 1.0);
		  glScalef(0.12, 0.03, 0.03);
		  glutSolidCube(1.0);
	   glPopMatrix();
	   glPushMatrix();
	   glTranslatef(-0.13, 0.23, 0.81);
		  glRotatef(30, 0.0, 0.0, 1.0);
		  glScalef(0.12, 0.03, 0.03);
		  glutSolidCube(1.0);
	   glPopMatrix();
	   //finger3
	   glPushMatrix();
		  glTranslatef(-0.07, 0.23, 0.75);
		  glRotatef(300, 0.0, 0.0, 1.0);
		  glScalef(0.12, 0.03, 0.03);
		  glutSolidCube(1.0);
		  
	   glPopMatrix();
	   glPushMatrix();
		  glTranslatef(-0.13, 0.23, 0.75);
		  glRotatef(30, 0.0, 0.0, 1.0);
		  glScalef(0.12, 0.03, 0.03);
		  glutSolidCube(1.0);
		  
	   glPopMatrix();
	   //finger down
	   glPushMatrix();
		  glTranslatef(-0.12, 0, 0.81);
		  glRotatef(30, 0.0, 0.0, 1.0);
		  glScalef(0.12, 0.03, 0.03);
		  glutSolidCube(1.0);
	   glPopMatrix();
	   glPushMatrix();
		  glTranslatef(-0.21, 0, 0.81);
		  glRotatef(300, 0.0, 0.0, 1.0);
		  glScalef(0.12, 0.03, 0.03);
		  glutSolidCube(1.0);
	   glPopMatrix();
    glPopMatrix();
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glPopMatrix();
    //planet
    glPushMatrix();
	   glTranslatef(0.4, 0.15, 0.35);
	   glColor3f(245.0/255.0, 162.0/255.0, 49.0/255.0);
	   //glutWireSphere(0.05, 10, 8);   /* draw sun */
	   glutSolidSphere(0.05, 16,16 );
	   //glColor3f(0.0, 0.0, 1.0);
	   glColor3f(3.0 / 255.0, 139.0 / 255.0, 217.0 / 255.0);
	   glRotatef((GLfloat)year, 0.0, 1.0, 0.0);
	   glTranslatef(0.1, 0.0, 0.0);
	   glRotatef((GLfloat)day, 0.0, 1.0, 0.0);
	   //glutWireSphere(0.015, 5, 4);    /* draw smaller planet */
	   glutSolidSphere(0.015,16,16);

	   glTranslatef(0.006, 0.0, 0.0);
	   glColor3f(2.0 / 255.0, 235.0 / 255.0, 111.0 / 255.0);
	   glutSolidSphere(0.01, 16, 16);
    glPopMatrix();
    glPopMatrix();
    
    
    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 20.0);
    glMatrixMode(GL_MODELVIEW);
}
static GLfloat R[9] = { 54,22,15,12,11,9,8 ,5,3 };
static GLfloat G[9] = { 235,240,240,215,202,184,167,141,110 };
static GLfloat Blu[9] = { 129,156,179,186,190,195,200,207,217 };
static GLint a = 0, u = 1,c=0;
void spinDisplay(void) {
    //logo
    spin = spin + 2.0;
    if (a <= 8 && a >= 0) {
	   if (c == 5) {
		  c = 0;
		  color[0] = R[a] / 255.0;
		  color[1] = G[a] / 255.0;
		  color[2] = Blu[a] / 255.0;
		  //cout << a << endl;
		  a = a + u;
	   }
	   else {
		  c = c + 1;
	   }
    }
    else {
	   u = -u;
	   a = a + u;
    }
    if (spin > 360.0)
	   spin = spin - 360.0;
    //planet
    day = (day + 4) % 360;
    year = (year + 2) % 360;
    glutPostRedisplay();
}
//static GLint m = 0;
void move(int m) {
    if (m == 1) {
	   meZ = meZ - 0.02;
	   seeZ = seeZ - 0.02;
    }
    else if (m == 2) {
	   meZ = meZ + 0.02;
	   seeZ = seeZ + 0.02;

    }
    else if (m == 3) {
	   meX = meX + 0.02;
	   seeX = seeX + 0.02;

    }
    else if (m == 4) {
	   meX = meX - 0.02;
	   seeX = seeX - 0.02;
    }
    glutPostRedisplay();
}
void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'w':
	   move(1);
	   break;
    case 's':
	   move(2);
	   break;
    case 'a':
	   move(4);
	   break;
    case 'd':
	   move(3);
	   break;
    case 27:
	   exit(0);
	   break;
    }
}
void catchKey(int key, int x, int y)
{
    if (key == GLUT_KEY_LEFT)
	   seeX = seeX - 0.02;
    else if (key == GLUT_KEY_RIGHT)
	   seeX = seeX + 0.02;
    else if (key == GLUT_KEY_DOWN)
	   seeY = seeY - 0.02;
    else if (key == GLUT_KEY_UP)
	   seeY = seeY + 0.02;
}
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    //glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(catchKey);
    glutIdleFunc(spinDisplay);
    glutMainLoop();
    return 0;
}