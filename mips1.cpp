#include <stdio.h>
#include <iostream>
#include <fstream>
#include <map>
#include <deque>
#include <string>
#include <cstring>
#include <cmath>
#include <queue>
using namespace std;

//调试
//ofstream fout("log.out");
//此处是内存大小的定义
const int M = 1024 * 1024;
const int SIZE = 4 * M; //调试结束后改回
//const int SIZE = 10000000;
//这是从文件读入时一行能放入的最大行数
const int MAX_SIZE = 5e3;

//此处为寄存器，注意还有两个低位，高位保存的寄存器
int rgstr[34] = { 0 };   //指定32号寄存器为低位寄存器，33号寄存器为高位寄存器

						 //此处为内存
char Memory[SIZE + 1] = { 0 };  //视情况决定是否改成unsigned
int fp = 0; int sp = SIZE;      //fp代表堆空间的帧指针，地址由低到高； sp代表栈空间的的栈指针，地址由高到低。
								//注意fp指针指向的是空的内存
								//此处是指令类型
map<string, int> label_local;
struct tmp_ins {
	int No = 0;
	short no_of_reg[3] = { 0 };
	int x = 0;    //bool x_exist = false;  //用于看x是否存在
	string label;                        //label的跳转信息，要进一步加工

										 //在此处设了初值
	tmp_ins() {
		for (int i = 0; i < 3; ++i) no_of_reg[i] = -1;
	}
	friend ostream& operator <<(ostream& os, const tmp_ins& obj) {
		os << "No: " << obj.No << "\n";
		for (int i = 0; i < 3; ++i) {
			os << "reg " << i << " : " << obj.no_of_reg[i] << "\n";
		}
		os << "x: " << obj.x << "\n";
		os << "label: " << obj.label << "\n";
		return os;
	}
};

struct instruction {
	int No;                 //操作类型,其中百位乃是其压进指令里的参数个数，千位是它所属的函数类别。
							//**1号寄存器专存写入的寄存器
	short no_of_reg[3];
	//三个会用到的寄存器的编号(其中一个寄存器在lw等操作的时候可能会存入原始的数据地址。如果没存入那意味着是用label所代表的位置访问）
	int x;      //bool x_exist;               //立即数或是偏移量（lw等操作）
	int label;                    //label的跳转序号,涉及指令的跳转和静态变量的访问

	instruction() {
		//x_exist = false;
	}
	instruction(const tmp_ins& obj) {
		No = obj.No;
		for (int i = 0; i < 3; ++i) no_of_reg[i] = obj.no_of_reg[i];
		x = obj.x;
		if (obj.label.length() == 0) label = -1;              //注意label为空也被改成了-1
		else label = label_local.find(obj.label)->second;
	}

	friend ostream& operator <<(ostream& os, const instruction& obj) {
		//os << "No: " << obj.No << "\n";
		for (int i = 0; i < 3; ++i) {
			os << "reg " << i << ": " << obj.no_of_reg[i] << "\t";
		}
		os << "x: " << obj.x << "\t";
		os << "label: " << obj.label << "\t";
		return os;
	}
};

//存放指令的地址
deque<tmp_ins> tmp_txt;
//deque<tmp_ins> text;
deque<instruction> text;
int main_start = 0;   //整个指令开始的地方

					  //基本映射操作的存储地方
map<string, int> opr;
map<string, int> reg;
map<string, int> data_opr;
map<int, string> map_out;

//从这里开始是数据读入部分
//完成数据输入和预处理，需要将整个代码读两遍    
void initialize();  //初始的map操作等。
void gt(char str[], char tmp[], int& count);
int to_int(char tmp[], int& count);

void align_(char tmp[], int & count, bool & deal_flag);
void ascii_(char tmp[], int & count, bool & deal_flag);
void asciiz_(char tmp[], int & count, bool & deal_flag);
void byte_(char tmp[], int & count, bool & deal_flag);
void half_(char tmp[], int & count, bool & deal_flag);
void word_(char tmp[], int & count, bool & deal_flag);
void space_(char tmp[], int & count, bool & deal_flag);
void data_(char tmp[], int & count, bool & deal_flag);
void text_(char tmp[], int & count, bool & deal_flag);

void(*deal_with_data[9])(char[], int &, bool &) = { align_, ascii_, asciiz_, byte_, half_, word_, space_, data_, text_ };

void form_label(char label[], int loca, bool flag, char tmp[], int& count);
void form_cmd_0(int num, char read[], int& count);
void form_cmd_1(int num, char read[], int& count);
void form_cmd_2(int num, char read[], int& count);
void form_cmd_3(int num, char read[], int& count);
void form_cmd_4(int num, char read[], int& count);
void form_cmd_5(int num, char read[], int& count);
void form_cmd_6(int num, char read[], int& count);
void form_cmd_7(int num, char read[], int& count);
void form_cmd_8(int num, char read[], int& count);
void form_cmd_9(int num, char read[], int& count);
void(*form_cmd[10])(int, char[], int&) = { form_cmd_0, form_cmd_1, form_cmd_2, form_cmd_3, form_cmd_4,
form_cmd_5, form_cmd_6, form_cmd_7, form_cmd_8, form_cmd_9 };
void form_instruction(char command[], int& loca, char tmp[], int& count);
void Input(char* a);

//这是模拟CPU操作开始的地方
//这是一个统一的存放五级流水暂存信息的地方，同时会有一个清空函数
//同时我还需要一个控制命令访问的全局参数，来模拟PC寄存器
class performer {
private:
	struct product {
		int instruct_address;                      //命令在文本中的位置，jal等可能会使用
												   //instruction 的 label 和 no_of_reg 如果没有，初值为-1
		instruction instruct;                      //命令本身，只会被读取，不会被修改(如果能够改成引用最好)
		int data_from_reg[3] = { 0 };              //可能会需要读取至多两个寄存器的内容（load命令）//第二阶段修改，第三阶段使用//可能是两个寄存器里的内容，也可能是一个偏移量 / 立即数，一个寄存器内容
		int reg_to_be_input[2] = { -1, -1 };             //需要被放入的寄存器的编号
		int ans_of_stage_3[2] = { 0, 0 };                 //第三步数据暂存, 可能是一个布尔变量，可能是一个内存地址，可能是算术数(低32位和高32位)
		int data_from_Memory = 0;                  //从第四步中读出来的数据
												   //label 就一直包含在命令里，不需要cpu在执行时额外存储
												   //注意上述所有信息中标0的都是在输入后无法确定的量
		void fresh() {
			for (int i = 0; i <= 1; ++i) {
				reg_to_be_input[i] = -1;
				ans_of_stage_3[i] = 0;
			}
		}
	};

	product order[5];    //前个阶段中每个阶段执行完之后的结果暂存。每一次执行，后一级使用前一级的暂存结果

						 //控制下一条要读取的命令
						 //PC寄存器可以用来确定第一阶段是否可以访问，（-1即为不可访问）
	int PC_order = -1;

	//以下为处理hazard所需要的一些数据
	int len;   //防止访问越界
	//此变量用于控制程序是否已经结束了
	bool finish = false;
	//此变量用于当前阶段是否处于control hazard
	short control_hazard = 0;
	//用于判断寄存器是否正在被使用	
	short use_reg[34] = { 0 };
	//此变量用于当前阶段是否处于data hazard
	short data_hazard = 0;
	//将阶段信息外存。此信息用来保存
	bool can_read[5] = { false };

	//一下是一些工具函数：
	//用于第二阶段的流水；
	//void other_prepare(product& obj);
	int reg_prepare(product& obj);
	//用于第三阶段
	void add(product& obj) {
		if (obj.instruct.no_of_reg[2] != -1) obj.ans_of_stage_3[0] = obj.data_from_reg[0] + obj.data_from_reg[1];
		else obj.ans_of_stage_3[0] = obj.data_from_reg[0] + obj.instruct.x;
	}
	void addu(product& obj) {
		if (obj.instruct.no_of_reg[2] != -1) obj.ans_of_stage_3[0] = obj.data_from_reg[0] + obj.data_from_reg[1];
		else obj.ans_of_stage_3[0] = obj.data_from_reg[0] + obj.instruct.x;
	}
	void addiu(product& obj) { obj.ans_of_stage_3[0] = obj.data_from_reg[0] + obj.instruct.x; }
	void sub(product& obj) {
		if (obj.instruct.no_of_reg[2] != -1) obj.ans_of_stage_3[0] = obj.data_from_reg[0] - obj.data_from_reg[1];
		else obj.ans_of_stage_3[0] = obj.data_from_reg[0] - obj.instruct.x;
	}
	void subu(product& obj) {
		if (obj.instruct.no_of_reg[2] != -1) obj.ans_of_stage_3[0] = obj.data_from_reg[0] - obj.data_from_reg[1];
		else obj.ans_of_stage_3[0] = obj.data_from_reg[0] - obj.instruct.x;
	}
	void mul(product& obj) {
		if (obj.instruct.no_of_reg[0] != -1) {
			if (obj.instruct.no_of_reg[2] != -1) obj.ans_of_stage_3[0] = obj.data_from_reg[0] * obj.data_from_reg[1];
			else obj.ans_of_stage_3[0] = obj.data_from_reg[0] * obj.instruct.x;
		}
		else {
			long long tmp;
			if (obj.instruct.no_of_reg[2] != -1) tmp = obj.data_from_reg[0] * obj.data_from_reg[1];
			else tmp = obj.data_from_reg[0] * obj.instruct.x;
			obj.ans_of_stage_3[0] = int(tmp);
			obj.ans_of_stage_3[1] = int(tmp >> 32);
		}
	}
	void mulu(product& obj) {
		if (obj.instruct.no_of_reg[0] != -1) {
			if (obj.instruct.no_of_reg[2] != -1) obj.ans_of_stage_3[0] = unsigned(obj.data_from_reg[0]) * unsigned(obj.data_from_reg[1]);
			else obj.ans_of_stage_3[0] = unsigned(obj.data_from_reg[0]) * unsigned(obj.instruct.x);
		}
		else {
			long long tmp;
			if (obj.instruct.no_of_reg[2] != -1) tmp = unsigned(obj.data_from_reg[0]) * unsigned(obj.data_from_reg[1]);
			else tmp = unsigned(obj.data_from_reg[0]) * unsigned(obj.instruct.x);
			obj.ans_of_stage_3[0] = int(tmp);
			obj.ans_of_stage_3[1] = int(tmp >> 32);
		}
	}
	void div(product& obj) {
		if (obj.instruct.no_of_reg[0] != -1) {
			if (obj.instruct.no_of_reg[2] != -1) obj.ans_of_stage_3[0] = obj.data_from_reg[0] / obj.data_from_reg[1];
			else obj.ans_of_stage_3[0] = obj.data_from_reg[0] / obj.instruct.x;
		}
		else {
			if (obj.instruct.no_of_reg[2] != -1) {
				obj.ans_of_stage_3[0] = obj.data_from_reg[0] / obj.data_from_reg[1];
				obj.ans_of_stage_3[1] = obj.data_from_reg[0] % obj.data_from_reg[1];
			}
			else {
				obj.ans_of_stage_3[0] = obj.data_from_reg[0] / obj.instruct.x;
				obj.ans_of_stage_3[1] = obj.data_from_reg[0] % obj.instruct.x;
			}
		}
	}
	void divu(product& obj) {
		if (obj.instruct.no_of_reg[0] != -1) {
			if (obj.instruct.no_of_reg[2] != -1) obj.ans_of_stage_3[0] = unsigned(obj.data_from_reg[0]) / unsigned(obj.data_from_reg[1]);
			else obj.ans_of_stage_3[0] = unsigned(obj.data_from_reg[0]) / unsigned(obj.instruct.x);
		}
		else {
			if (obj.instruct.no_of_reg[2] != -1) {
				obj.ans_of_stage_3[0] = unsigned(obj.data_from_reg[0]) / unsigned(obj.data_from_reg[1]);
				obj.ans_of_stage_3[1] = unsigned(obj.data_from_reg[0]) % unsigned(obj.data_from_reg[1]);
			}
			else {
				obj.ans_of_stage_3[0] = unsigned(obj.data_from_reg[0]) / unsigned(obj.instruct.x);
				obj.ans_of_stage_3[1] = unsigned(obj.data_from_reg[0]) % unsigned(obj.instruct.x);
			}
		}
	}
	void xor_(product& obj) {
		if (obj.instruct.no_of_reg[2] != -1) obj.ans_of_stage_3[0] = obj.data_from_reg[0] ^ obj.data_from_reg[1];
		else obj.ans_of_stage_3[0] = obj.data_from_reg[0] ^ obj.instruct.x;
	}
	void xoru(product& obj) {
		if (obj.instruct.no_of_reg[2] != -1) obj.ans_of_stage_3[0] = unsigned(obj.data_from_reg[0]) / unsigned(obj.data_from_reg[1]);
		else obj.ans_of_stage_3[0] = unsigned(obj.data_from_reg[0]) / unsigned(obj.instruct.x);
	}
	void neg(product& obj) { obj.ans_of_stage_3[0] = -obj.data_from_reg[0]; }     //neg是取负号吗？
	void negu(product& obj) { obj.ans_of_stage_3[0] = unsigned(-obj.data_from_reg[0]); }
	void rem(product& obj) {
		if (obj.instruct.no_of_reg[2] != -1) obj.ans_of_stage_3[0] = obj.data_from_reg[0] % obj.data_from_reg[1];
		else obj.ans_of_stage_3[0] = obj.data_from_reg[0] % obj.instruct.x;
	}
	void remu(product& obj) {
		if (obj.instruct.no_of_reg[2] != -1) obj.ans_of_stage_3[0] = unsigned(obj.data_from_reg[0]) % unsigned(obj.data_from_reg[1]);
		else obj.ans_of_stage_3[0] = unsigned(obj.data_from_reg[0]) % unsigned(obj.instruct.x);
	}
	void li(product& obj) { obj.ans_of_stage_3[0] = obj.instruct.x; }
	void seq(product& obj) {
		if (obj.instruct.no_of_reg[2] != -1) obj.ans_of_stage_3[0] = (obj.data_from_reg[0] == obj.data_from_reg[1]);
		else obj.ans_of_stage_3[0] = (obj.data_from_reg[0] == obj.instruct.x);
	}
	void sge(product& obj) {
		if (obj.instruct.no_of_reg[2] != -1) obj.ans_of_stage_3[0] = (obj.data_from_reg[0] >= obj.data_from_reg[1]);
		else obj.ans_of_stage_3[0] = (obj.data_from_reg[0] >= obj.instruct.x);
	}
	void sgt(product& obj) {
		if (obj.instruct.no_of_reg[2] != -1) obj.ans_of_stage_3[0] = (obj.data_from_reg[0] > obj.data_from_reg[1]);
		else obj.ans_of_stage_3[0] = (obj.data_from_reg[0] > obj.instruct.x);
	}
	void sle(product& obj) {
		if (obj.instruct.no_of_reg[2] != -1) obj.ans_of_stage_3[0] = (obj.data_from_reg[0] <= obj.data_from_reg[1]);
		else obj.ans_of_stage_3[0] = (obj.data_from_reg[0] <= obj.instruct.x);
	}
	void slt(product& obj) {
		if (obj.instruct.no_of_reg[2] != -1) obj.ans_of_stage_3[0] = (obj.data_from_reg[0] < obj.data_from_reg[1]);
		else obj.ans_of_stage_3[0] = (obj.data_from_reg[0] < obj.instruct.x);
	}
	void sne(product& obj) {
		if (obj.instruct.no_of_reg[2] != -1) obj.ans_of_stage_3[0] = (obj.data_from_reg[0] != obj.data_from_reg[1]);
		else obj.ans_of_stage_3[0] = (obj.data_from_reg[0] != obj.instruct.x);
	}
	void b(product& obj) { obj.ans_of_stage_3[0] = true; }
	void beq(product& obj) { seq(obj); }
	void bne(product& obj) { sne(obj); }
	void bge(product& obj) { sge(obj); }
	void ble(product& obj) { sle(obj); }
	void bgt(product& obj) { sgt(obj); }
	void blt(product& obj) { slt(obj); }

	void beqz(product& obj) { obj.instruct.x = 0; seq(obj); }
	void bnez(product& obj) { obj.instruct.x = 0; sne(obj); }
	void blez(product& obj) { obj.instruct.x = 0; sle(obj); }
	void bgez(product& obj) { obj.instruct.x = 0; sge(obj); }
	void bgtz(product& obj) { obj.instruct.x = 0; sgt(obj); }
	void bltz(product& obj) { obj.instruct.x = 0; slt(obj); }
	void j(product& obj) { obj.ans_of_stage_3[0] = true; }
	void jr(product& obj) { obj.ans_of_stage_3[0] = true; }
	void jal(product& obj) { obj.ans_of_stage_3[0] = true; }
	void jalr(product& obj) { obj.ans_of_stage_3[0] = true; }
	void la(product& obj) {
		if (obj.instruct.label >= 0) obj.ans_of_stage_3[0] = obj.instruct.label;
		else obj.ans_of_stage_3[0] = obj.data_from_reg[0] + obj.instruct.x;
	}
	void lb(product& obj) { la(obj); }
	void lh(product& obj) { la(obj); }
	void lw(product& obj) { la(obj); }
	void sb(product& obj) {
		if (obj.instruct.label >= 0) obj.ans_of_stage_3[0] = obj.instruct.label;
		else obj.ans_of_stage_3[0] = obj.data_from_reg[1] + obj.instruct.x;
	}
	void sh(product& obj) { sb(obj); }
	void sw(product& obj) { sb(obj); }
	void move(product& obj) { obj.ans_of_stage_3[0] = obj.data_from_reg[0]; }
	void mfhi(product& obj) { obj.ans_of_stage_3[0] = obj.data_from_reg[0]; }
	void mflo(product& obj) { obj.ans_of_stage_3[0] = obj.data_from_reg[0]; }
	void nop(product& obj) { ; }
	void syscall(product& obj);   //注意这个指令有可能修改finish的值！

	void (performer::*excute[52]) (product&) = { NULL, &performer::add, &performer::addu, &performer::addiu, &performer::sub,
		&performer::subu, &performer::mul, &performer::mulu, &performer::div, &performer::divu, &performer::xor_, &performer::xoru,
		&performer::neg, &performer::negu, &performer::rem, &performer::remu, &performer::li, &performer::seq, &performer::sge,
		&performer::sgt, &performer::sle, &performer::slt, &performer::sne, &performer::b, &performer::beq, &performer::bne,
		&performer::bge, &performer::ble, &performer::bgt, &performer::blt, &performer::beqz, &performer::bnez, &performer::blez,
		&performer::bgez, &performer::bgtz, &performer::bltz, &performer::j, &performer::jr, &performer::jal, &performer::jalr,
		&performer::la, &performer::lb, &performer::lh, &performer::lw, &performer::sb, &performer::sh, &performer::sw, &performer::move,
		&performer::mfhi, &performer::mflo, &performer::nop, &performer::syscall };

public:
	performer(int start) {
		PC_order = start;
		can_read[0] = true;
		len = text.size();
	}
	bool end() { return finish; }

	//注意：第五阶段不再需要考虑下一级是否能访问
	void Write_Bk() {
		if (!finish && can_read[4]) {
			//写回操作
			//写完记得设置use_reg
			order[4] = order[3];

			int num = order[4].instruct.No % 100;
			if (23 <= num && num <= 39) {   //跳转指令
				--control_hazard;
				if (order[4].ans_of_stage_3[0]) {
					if (order[4].instruct.label != -1) PC_order = order[4].instruct.label;//确保此时第一阶段还没有被syscall修改为不可访问
					else PC_order = order[4].data_from_reg[0];

					if (38 <= num && num <= 39) {  //如果是jal/jalr
						rgstr[order[4].reg_to_be_input[0]] = order[4].instruct_address + 1;
						--use_reg[order[4].reg_to_be_input[0]];
					}
				}
			}
			else if (40 <= num && num <= 43) {  //load 命令
				rgstr[order[4].reg_to_be_input[0]] = order[4].data_from_Memory;
				--use_reg[order[4].reg_to_be_input[0]];
			}
			else {
				if (num == 51) {           //syscall可能结束整个命令
					if (order[4].data_from_reg[2] == 10 || order[4].data_from_reg[2] == 17) {
						finish = true;
					}
				}
				for (int i = 0; i <= 1; ++i) {
					if (order[4].reg_to_be_input[i] != -1) {
						rgstr[order[4].reg_to_be_input[i]] = order[4].ans_of_stage_3[i];
						--use_reg[order[4].reg_to_be_input[i]];
					}
				}
			}
		}
	}

	void Memory_Access() {
		if (!finish && can_read[3]) {
			order[3] = order[2];
			int num = order[2].instruct.No % 100;
			if (40 <= num && num <= 43) {    //load命令
				if (num == 40) order[3].data_from_Memory = order[3].ans_of_stage_3[0];
				else if (num == 41) order[3].data_from_Memory = Memory[order[3].ans_of_stage_3[0]];
				else if (num == 42) order[3].data_from_Memory = *((short*)(Memory + order[3].ans_of_stage_3[0]));
				else if (num == 43) order[3].data_from_Memory = *((int*)(Memory + order[3].ans_of_stage_3[0]));

			}
			else if (44 <= num && num <= 46) {  //store
				if (num == 44) Memory[order[3].ans_of_stage_3[0]] = order[3].data_from_reg[0];
				else if (num == 45) new (static_cast<void*> (&Memory[order[3].ans_of_stage_3[0]])) short(order[3].data_from_reg[0]);  //访问内存
				else if (num == 46) new (static_cast<void*> (&Memory[order[3].ans_of_stage_3[0]])) int(order[3].data_from_reg[0]);    //访问内存
			}
			else if (num == 51) {
				if (order[3].ans_of_stage_3[1] == -10) {
					char tmp[2000];
					cin >> tmp;
					int len = strlen(tmp);
					int address = order[3].data_from_reg[0];
					for (int i = 0; i <= len; ++i) {
						new (static_cast<void*> (&Memory[address++])) char(tmp[i]);    //访问内存了
					}
				}
			}
			can_read[4] = true;
		}
		else can_read[4] = false;
		//load: 把内存地址的数据取出
		//store : 将数据写入内存中（这个时候数据早已经从寄存器中读了出来
	}

	void Execution() {
		if (!finish && can_read[2]) {
			order[2] = order[1];
			int num = order[2].instruct.No % 100;
			(this->*excute[num])(order[2]);//类中使用函数指针的样式

			can_read[3] = true;
		}
		else can_read[3] = false;
	}

	void Decode_Pre() {
		if (!finish) {
			if (!data_hazard && !control_hazard && can_read[1]) {
				order[1] = order[0];

				//other_prepare(order[1]);   //非syscall的命令在此阶段完成确定哪些寄存器要被写入

				//0：非控制且读取失败， 1：既是控制命令又读取失败， 2：控制且读取成功，3：非控制且读取成功。
				int stage = reg_prepare(order[1]);         //用于后续区别；

				if (stage == 0) {            //读取失败
					++data_hazard;
					can_read[2] = false;
				}
				else if (stage == 2) {
					++control_hazard;
					can_read[2] = true;
				}
				else if (stage == 1) {
					++control_hazard;
					++data_hazard;
					can_read[2] = false;
				}
				else can_read[2] = true;
			}
			else {
				//无论是否control_hazard，至少数据都可以往上递了（can_read[1]都可置为1了）
				if (data_hazard) {
					//访问寄存器成功
					if (reg_prepare(order[1]) >= 2) {
						--data_hazard;
						can_read[2] = true;
					}
					else can_read[2] = false;
				}
				else can_read[2] = false;  //仍旧处于不可读取阶段
			}
		}
		else can_read[2] = false;
	}

	void Ins_Fetch() {
		//诸如已经发出结束命令之类的就不再load。hazard等也是需要考
		if (can_read[0] && !finish && !control_hazard && !data_hazard && PC_order < len) {
			order[0].fresh();
			order[0].instruct = text[PC_order];
			order[0].instruct_address = PC_order;

			//fout << "PC_order: " << PC_order << " order: " << map_out[order[0].instruct.No % 100] << "\n";// << " " << order[0].instruct << "\n";
			/*fout << PC_order << " " << map_out[order[0].instruct.No % 100] << "\t" << order[0].instruct << "\n";
			for (int i = 0; i <= 33; ++i) {
			fout << i << "\t";
			}
			fout << "\n";
			for (int i = 0; i <= 33; ++i) {
			fout << rgstr[i] << "\t";
			}
			fout << "\n";// << "\n" << "\n";
			/*if (PC_order == 587) {
			/*finish = true;
			can_read[1] = false;
			return;
			system("pause");
			}*/
			++PC_order;
			can_read[1] = true;
		}
		else {
			can_read[1] = false;
		}
	}
};

int performer::reg_prepare(performer::product& obj) {   //记得改回引用
	int state;
	int num = (obj.instruct.No) % 100;
	if (num == 51) {
		//syscall
		if (use_reg[reg["$v0"]]) return 0;
		else {
			int choice = rgstr[reg["$v0"]];
			obj.data_from_reg[2] = choice;
			if (choice == 1 || choice == 4) {
				if (use_reg[reg["$a0"]]) return 0;
				else {
					obj.data_from_reg[0] = rgstr[reg["$a0"]];
					//use_reg[reg["$a0"]] = true;
					//use_reg[reg["$v0"]] = true;
					return 3;
				}
			}
			else if (choice == 5) {
				obj.reg_to_be_input[0] = reg["$v0"];
				++use_reg[reg["$v0"]];
				return 3;
			}
			else if (choice == 8) {
				if (use_reg[reg["$a0"]] || use_reg[reg["$a1"]]) return 0;
				else {
					//use_reg[reg["$a0"]] = true;  use_reg[reg["$a1"]] = true;
					//use_reg[reg["$v0"]] = true;
					obj.data_from_reg[0] = rgstr[reg["$a0"]];
					obj.data_from_reg[1] = rgstr[reg["$a1"]];
					return 3;
				}
			}
			else if (choice == 9) {
				if (use_reg[reg["$a0"]]) return 0;
				else {
					obj.data_from_reg[0] = rgstr[reg["$a0"]];
					obj.reg_to_be_input[0] = reg["$v0"];
					//use_reg[reg["$a0"]] = true;
					++use_reg[reg["$v0"]];
					return 3;
				}
			}
			else if (choice == 10) {
				//use_reg[reg["$v0"]] = true;
				can_read[0] = false;
				return 3;
			}
			else if (choice == 17) {
				if (use_reg[reg["$a0"]]) {
					can_read[0] = false;
					return 0;
				}
				else {
					obj.data_from_reg[0] = rgstr[reg["$a0"]];
					//use_reg[reg["$a0"]] = true;
					//use_reg[reg["$v0"]] = true;
					return 3;
				}
			}
		}
	}
	else if (23 <= num && num <= 39) {//跳转命令
		for (int i = 1; i <= 2; ++i) {
			if (obj.instruct.no_of_reg[i] != -1 && use_reg[obj.instruct.no_of_reg[i]]) return 1;
		}
		//此为将数据读入
		for (int i = 1; i <= 2; ++i) {
			//use_reg[obj.instruct.no_of_reg[i]] = true;
			if (obj.instruct.no_of_reg[i] != -1) obj.data_from_reg[i - 1] = rgstr[obj.instruct.no_of_reg[i]];
		}
		//此为将被写的寄存器准备好
		if (num == 38 || num == 39) {    //jal命令
			obj.reg_to_be_input[0] = 31;
			++use_reg[obj.reg_to_be_input[0]];
		}
		/*else if (num == 39) {    //jalr命令
		obj.reg_to_be_input[0] = 31;
		//obj.reg_to_be_input[1] = obj.instruct.no_of_reg[0]; //??这条命令是做什么的？
		use_reg[obj.reg_to_be_input[0]] = true;
		//use_reg[obj.reg_to_be_input[1]] = true;
		}*/
		//跳转系除了上面几条均不需要有写入寄存器操作
		/*else if (obj.instruct.no_of_reg[0] != -1) {
		obj.reg_to_be_input[0] = obj.instruct.no_of_reg[0];
		use_reg[obj.reg_to_be_input[0]] = true;
		}*/
		return 2;
	}
	else if (6 <= num && num <= 9) {                      //乘法或除法命令
		for (int i = 1; i <= 2; ++i) {
			if (obj.instruct.no_of_reg[i] != -1 && use_reg[obj.instruct.no_of_reg[i]]) return 0;
		}
		//此为将数据读入
		for (int i = 1; i <= 2; ++i) {
			//use_reg[obj.instruct.no_of_reg[i]] = true;
			if (obj.instruct.no_of_reg[i] != -1) obj.data_from_reg[i - 1] = rgstr[obj.instruct.no_of_reg[i]];
		}
		//确定写入的寄存器
		if (obj.instruct.no_of_reg[0] == -1) {       //没有指定写入的寄存器(0号寄存器就是用来写入的)
			obj.reg_to_be_input[0] = 32;
			obj.reg_to_be_input[1] = 33;
			++use_reg[32]; ++use_reg[33];
		}
		else {
			obj.reg_to_be_input[0] = obj.instruct.no_of_reg[0];
			++use_reg[obj.reg_to_be_input[0]];
		}
		return 3;
	}
	else if (num == 48) {//mfhi
		if (use_reg[33]) return 0;
		else {
			//use_reg[33] = true;
			//读入数据
			obj.data_from_reg[0] = rgstr[33];
			//准备写入的寄存器
			obj.reg_to_be_input[0] = obj.instruct.no_of_reg[0];
			++use_reg[obj.reg_to_be_input[0]];
			return 3;
		}
	}
	else if (num == 49) {//mflo
		if (use_reg[32]) return 0;
		else {
			//use_reg[32] = true;
			obj.data_from_reg[0] = rgstr[32];
			obj.reg_to_be_input[0] = obj.instruct.no_of_reg[0];
			++use_reg[obj.reg_to_be_input[0]];
			return 3;
		}
	}
	else {
		for (int i = 1; i <= 2; ++i) {
			if (obj.instruct.no_of_reg[i] != -1 && use_reg[obj.instruct.no_of_reg[i]]) return 0;
		}
		//此为将数据读入
		for (int i = 1; i <= 2; ++i) {
			//use_reg[obj.instruct.no_of_reg[i]] = true;
			if (obj.instruct.no_of_reg[i] != -1) obj.data_from_reg[i - 1] = rgstr[obj.instruct.no_of_reg[i]];
		}
		//确定写入的寄存器
		if (obj.instruct.no_of_reg[0] != -1) {
			obj.reg_to_be_input[0] = obj.instruct.no_of_reg[0];
			++use_reg[obj.reg_to_be_input[0]];
		}
		return 3;
	}
}

//处理的是： 立即数/偏移量 + 要写入的寄存器
//syscall的第三步实现
void performer::syscall(performer::product& obj) {  //记得改回引用
	int loca;
	switch (obj.data_from_reg[2])
	{
	case 1:cout << obj.data_from_reg[0]; break;   //先加换行符，方便调试
	case 4:
		loca = obj.data_from_reg[0];
		while (Memory[loca] != '\0') {
			cout << Memory[loca++];
		}
		break;
	case 5:cin >> obj.ans_of_stage_3[0]; break;
	case 8:
		obj.ans_of_stage_3[1] = -10; break;//取一个特殊值来使其在第4阶段执行内存访问
	case 9:
		if (fp % 4 != 0) {
			int k = fp / 4 + 1;
			fp = k * 4;
		}
		obj.ans_of_stage_3[0] = fp;
		fp += obj.data_from_reg[0];
		break;
	case 10: case 17: break;
	}
}

//完成整个模拟操作
void Performance();

int main(int argc, char** argv)
{
	//ofstream out("log.txt");

	Input(argv[1]);

	rgstr[reg["$sp"]] = sp;                     //在静态变量赋值结束以后马上将栈空间的最后一个赋值给他。在流水线开始之前
	main_start = label_local["main"];   //确定整个指令开始的地方。

	Performance();

	//out.close();
	//system("pause");
	return 0;
}

//完成基本的map操作
//编码涉及你对于五级架构的设想
void initialize() {
	reg["$zero"] = 0; reg["$0"] = 0;
	reg["$at"] = 1; reg["$1"] = 1;
	reg["$v0"] = 2; reg["$2"] = 2;
	reg["$v1"] = 3; reg["$3"] = 3;
	reg["$a0"] = 4; reg["$4"] = 4;
	reg["$a1"] = 5; reg["$5"] = 5;
	reg["$a2"] = 6; reg["$6"] = 6;
	reg["$a3"] = 7; reg["$7"] = 7;
	reg["$t0"] = 8; reg["$8"] = 8;
	reg["$t1"] = 9; reg["$9"] = 9;
	reg["$t2"] = 10; reg["$10"] = 10;
	reg["$t3"] = 11; reg["$11"] = 11;
	reg["$t4"] = 12; reg["$12"] = 12;
	reg["$t5"] = 13; reg["$13"] = 13;
	reg["$t6"] = 14; reg["$14"] = 14;
	reg["$t7"] = 15; reg["$15"] = 15;
	reg["$s0"] = 16; reg["$16"] = 16;
	reg["$s1"] = 17; reg["$17"] = 17;
	reg["$s2"] = 18; reg["$18"] = 18;
	reg["$s3"] = 19; reg["$19"] = 19;
	reg["$s4"] = 20; reg["$20"] = 20;
	reg["$s5"] = 21; reg["$21"] = 21;
	reg["$s6"] = 22; reg["$22"] = 22;
	reg["$s7"] = 23; reg["$23"] = 23;
	reg["$t8"] = 24; reg["$24"] = 24;
	reg["$t9"] = 25; reg["$25"] = 25;
	reg["$k0"] = 26; reg["$26"] = 26;
	reg["$k1"] = 27; reg["$27"] = 27;
	reg["$gp"] = 28; reg["$28"] = 28;
	reg["$sp"] = 29; reg["$29"] = 29;
	reg["$s8"] = 30; reg["$fp"] = 30; reg["$30"] = 30;
	reg["$ra"] = 31; reg["$31"] = 31;

	opr["add"] = 21301; opr["addu"] = 21302;
	opr["addiu"] = 21303; opr["sub"] = 21304;
	opr["subu"] = 21305; opr["mul"] = 31506;
	opr["mulu"] = 31507; opr["div"] = 31508;
	opr["divu"] = 31509; opr["xor"] = 21310;
	opr["xoru"] = 21311; opr["neg"] = 11212;
	opr["negu"] = 11213; opr["rem"] = 21314;
	opr["remu"] = 21315;
	opr["li"] = 92216;
	opr["seq"] = 23317; opr["sge"] = 23318;
	opr["sgt"] = 23319; opr["sle"] = 23320;
	opr["slt"] = 23321; opr["sne"] = 23322;
	opr["b"] = 44123; opr["beq"] = 54324;
	opr["bne"] = 54325; opr["bge"] = 54326;
	opr["ble"] = 54327; opr["bgt"] = 54328;
	opr["blt"] = 54329; opr["beqz"] = 64230;
	opr["bnez"] = 64231; opr["blez"] = 64232;
	opr["bgez"] = 64233; opr["bgtz"] = 64234;
	opr["bltz"] = 64235; opr["j"] = 44136;
	opr["jr"] = 44137; opr["jal"] = 44138;
	opr["jalr"] = 44139;
	opr["la"] = 75340; opr["lb"] = 75341;
	opr["lh"] = 75342; opr["lw"] = 75343;
	opr["sb"] = 86344; opr["sh"] = 86345;
	opr["sw"] = 86346;
	opr["move"] = 97247; opr["mfhi"] = 97148;
	opr["mflo"] = 97149;
	opr["nop"] = 8050;
	opr["syscall"] = 9051;

	data_opr[".align"] = 0; data_opr[".ascii"] = 1;
	data_opr[".asciiz"] = 2; data_opr[".byte"] = 3;
	data_opr[".half"] = 4; data_opr[".word"] = 5;
	data_opr[".space"] = 6; data_opr[".data"] = 7;
	data_opr[".text"] = 8;

	map_out[1] = "add"; map_out[2] = "addu";
	map_out[3] = "addiu"; map_out[4] = "sub";
	map_out[5] = "subu"; map_out[6] = "mul";
	map_out[7] = "mulu"; map_out[8] = "div";
	map_out[9] = "divu"; map_out[10] = "xor";
	map_out[11] = "xoru"; map_out[12] = "neg";
	map_out[13] = "negu"; map_out[14] = "rem";
	map_out[15] = "remu"; map_out[16] = "li";
	map_out[17] = "seq"; map_out[18] = "sge";
	map_out[19] = "sgt"; map_out[20] = "sle";
	map_out[21] = "slt"; map_out[22] = "sne";
	map_out[23] = "b"; map_out[24] = "beq";
	map_out[25] = "bne"; map_out[26] = "bge";
	map_out[27] = "ble"; map_out[28] = "bgt";
	map_out[29] = "blt"; map_out[30] = "beqz";
	map_out[31] = "bnez"; map_out[32] = "blez";
	map_out[33] = "bgez"; map_out[34] = "bgtz";
	map_out[35] = "bltz"; map_out[36] = "j";
	map_out[37] = "jr"; map_out[38] = "jal";
	map_out[39] = "jalr"; map_out[40] = "la";
	map_out[41] = "lb"; map_out[42] = "lh";
	map_out[43] = "lw"; map_out[44] = "sb";
	map_out[45] = "sh"; map_out[46] = "sw";
	map_out[47] = "move"; map_out[48] = "mfhi";
	map_out[49] = "mflo"; map_out[50] = "nop";
	map_out[51] = "syscall";
}

//完成数据输入和预处理，需要将整个代码读两遍
void Input(char * a)
{
	/*string filename;
	cin >> filename;
	ifstream fin(filename);
	*/
	ifstream fin(a);
	//ifstream fin("test_loop.s");
	initialize();

	char tmp[MAX_SIZE];
	bool deal_flag = true;  //false 表示正在处理data区的数据， true 表示正在处理text区的数据
	int loca = 0;           //用来统计当前命令条数

	map<string, int>::iterator opr_itr;
	while (fin.getline(tmp, MAX_SIZE)) {
		int count = 0;
		int len = strlen(tmp);       //len表示tmp的字符串长度
		while (count < len) {
			char str[500];           //读入一条可能是命令，可能是label的字符段

			while ((tmp[count] == ' ' || tmp[count] == '\t' || tmp[count] == '\0') && count < len) ++count;
			//while (tmp[count] == ' ' || tmp[count] == '\t' || tmp[count] == ',') ++count;
			if (count >= len) break;

			if (tmp[count] == '#') break;
			gt(str, tmp, count);
			if (str[0] == '.') {
				deal_with_data[data_opr[str]](tmp, count, deal_flag); break;
			}
			else {
				opr_itr = opr.find(str);
				if (opr_itr == opr.end()) {
					form_label(str, loca, deal_flag, tmp, count);
				}
				else {
					//注意使用迭代器 -》迭代器可以给你对应的函数操作码
					form_instruction(str, loca, tmp, count); break;
					//在完成了命令形成以后这一行一定没有东西了
				}
			}
		}
	}

	deque<tmp_ins>::iterator txt_itr = tmp_txt.begin();
	for (; txt_itr != tmp_txt.end(); ++txt_itr) {
		text.push_back(*txt_itr);
		//cout << *txt_itr << "\n";
	}

	/*deque<instruction>::iterator text_itr = text.begin();
	for (; text_itr != text.end(); ++text_itr) {
	cout << *text_itr << "\n";
	}*/
	fin.close();
}

void Performance()
{
	performer CPU(main_start);
	while (!CPU.end()) {
		CPU.Write_Bk();
		CPU.Memory_Access();
		CPU.Execution();
		CPU.Decode_Pre();
		CPU.Ins_Fetch();
		//测试代码
		/*CPU.Ins_Fetch();
		CPU.Decode_Pre();
		CPU.Execution();
		CPU.Memory_Access();
		CPU.Write_Bk();*/
	}
}

//注意gt函数是跳空格的，但只能是针对后面一定有命令的时候！检查所有gt函数
//一旦调用gt函数，意味着在你的命令之前的信息对于你都已经没有作用了
//啥命令没读到给str一个NULL
void gt(char str[], char tmp[], int & count)
{
	int i = 0; int len = strlen(tmp);
	while ((tmp[count] == ' ' || tmp[count] == '\t' || tmp[count] == ',' || tmp[count] == '\0' || tmp[count] == '\"' || tmp[count] == '(') && count < len) ++count;

	bool use_flag = false;
	//可以考虑改进关于‘#’的内容
	while (tmp[count] != ' ' && tmp[count] != '\t' && tmp[count] != ',' && tmp[count] != ':' && tmp[count] != ')' && tmp[count] != '\0' && tmp[count] != '"' && count < len) {
		use_flag = true;
		str[i++] = tmp[count++];
	}
	str[i] = NULL;
	/*while (tmp[count] == '\0' || tmp[count] == ' ' || tmp[count] == '\t' )
	if (tmp[count] == ':' || tmp[count] == ')' || tmp[count] == '"') ++count;*/
}

//to_int 也是跳空格函数，注意访问时务必保证后面有数字
int to_int(char tmp[], int& count)
{
	int ans = 0;
	bool flag = false;
	int len = strlen(tmp);
	while ((tmp[count] == ' ' || tmp[count] == '\t' || tmp[count] == ',' || tmp[count] == '\0' || tmp[count] == '\"' || tmp[count] == '(') && count < len) ++count;
	if (tmp[count] == '-') {
		flag = true; ++count;
	}
	while ('0' <= tmp[count] && tmp[count] <= '9') {
		ans = ans * 10 + tmp[count++] - '0';
	}
	if (!flag) return ans;
	else return -ans;
}

void align_(char tmp[], int & count, bool & deal_flag)
{
	int num = to_int(tmp, count);
	int base = pow(2, num);
	if (fp % base != 0) {
		int k = fp / base + 1;
		fp = k * base;
	}
}
void ascii_(char tmp[], int & count, bool & deal_flag)
{
	while (tmp[count] != '"') ++count;
	++count;
	while (tmp[count] != '"') {
		if (tmp[count] == '\\') {
			switch (tmp[++count]) {
			case 'a': Memory[fp++] = '\a'; break;
			case 'b': Memory[fp++] = '\b'; break;
			case 'f': Memory[fp++] = '\f'; break;
			case 'n': Memory[fp++] = '\n'; break;
			case 'r': Memory[fp++] = '\r'; break;
			case 't': Memory[fp++] = '\t'; break;
			case 'v': Memory[fp++] = '\v'; break;
			case '0': Memory[fp++] = '\0'; break;
			case '\\': Memory[fp++] = '\\'; break;
			case '\'': Memory[fp++] = '\''; break;
			case '"': Memory[fp++] = '"'; break;
			}
		}
		else Memory[fp++] = tmp[count];
		++count;
	}
}
void asciiz_(char tmp[], int & count, bool & deal_flag)
{
	while (tmp[count] != '"') ++count;
	++count;
	while (tmp[count] != '"') {
		if (tmp[count] == '\\') {
			switch (tmp[++count]) {
			case 'a': Memory[fp++] = '\a'; break;
			case 'b': Memory[fp++] = '\b'; break;
			case 'f': Memory[fp++] = '\f'; break;
			case 'n': Memory[fp++] = '\n'; break;
			case 'r': Memory[fp++] = '\r'; break;
			case 't': Memory[fp++] = '\t'; break;
			case 'v': Memory[fp++] = '\v'; break;
			case '0': Memory[fp++] = '\0'; break;
			case '\\': Memory[fp++] = '\\'; break;
			case '\'': Memory[fp++] = '\''; break;
			case '"': Memory[fp++] = '"'; break;
			}
		}
		else Memory[fp++] = tmp[count];
		++count;
	}
	//count++;//
	Memory[fp++] = NULL;
}

void byte_(char tmp[], int & count, bool & deal_flag)
{
	int num;
	char str[500];
	do {
		gt(str, tmp, count);
		if (str[0] != NULL && str[0] != '#') {
			int a = 0;
			num = to_int(str, a);
			new (static_cast<void*> (&Memory[fp])) int(num);  //访问内存
			fp += 1;
		}
	} while (str[0] != NULL && str[0] != '#');
}

void half_(char tmp[], int & count, bool & deal_flag)
{
	int num;
	char str[500];
	do {
		gt(str, tmp, count);
		if (str[0] != NULL && str[0] != '#') {
			int a = 0;
			num = to_int(str, a);
			new (static_cast<void*> (&Memory[fp])) int(num);  //访问内存
			fp += 2;
		}
	} while (str[0] != NULL && str[0] != '#');
}

void word_(char tmp[], int & count, bool & deal_flag)
{
	int num;
	char str[500];
	do {
		gt(str, tmp, count);
		if (str[0] != NULL && str[0] != '#') {
			int a = 0;
			num = to_int(str, a);
			new (static_cast<void*> (&Memory[fp])) int(num);  //访问内存
			fp += 4;
		}
	} while (str[0] != NULL && str[0] != '#');
}

void space_(char tmp[], int & count, bool & deal_flag) { fp += to_int(tmp, count); }
void data_(char tmp[], int & count, bool & deal_flag) { deal_flag = false; }
void text_(char tmp[], int & count, bool & deal_flag) { deal_flag = true; }

//form_label函数还有为自己在指令读入上“擦屁股”的义务
void form_label(char label[], int loca, bool flag, char tmp[], int& count)
{
	if (!flag) label_local[label] = fp;  //此时处理的是静态变量
	else label_local[label] = loca;      //此处是指令的跳转到的地址

	int len = strlen(tmp);
	while ((tmp[count] == ' ' || tmp[count] == '\t' || tmp[count] == '\0') && count < len) ++count;
	if (tmp[count] == ':') ++count;
}

void form_instruction(char command[], int& loca, char tmp[], int& count)
{
	int num = opr[command];   //以此做一个思路向导。
	form_cmd[(num / 10000) % 10](num, tmp, count);
	++loca;
}

//检查一下寄存器都放好了没有
//无参数。
void form_cmd_0(int num, char read[], int& count)
{
	tmp_ins tmp;
	tmp.No = num;
	tmp_txt.push_back(tmp);
}

//两个寄存器 || 一个寄存器 + 立即数
void form_cmd_1(int num, char read[], int& count)
{
	tmp_ins tmp;
	tmp.No = num;
	char str[10];
	for (int i = 0; i < 2; ++i) {
		gt(str, read, count);
		tmp.no_of_reg[i] = reg[str];
	}
	tmp_txt.push_back(tmp);
}

//两个寄存器 + 一个寄存 / 立即数
void form_cmd_2(int num, char read[], int& count)
{
	tmp_ins tmp;
	tmp.No = num;
	char str[500];
	for (int i = 0; i < 2; ++i) {
		gt(str, read, count);
		tmp.no_of_reg[i] = reg[str];
	}

	gt(str, read, count);
	map<string, int>::iterator reg_itr;
	reg_itr = reg.find(str);
	int a = 0;   //字符数组的访问位置
	if (reg_itr == reg.end()) {
		tmp.x = to_int(str, a);
		//tmp.x_exist = true;
	}
	else tmp.no_of_reg[2] = reg[str];
	tmp_txt.push_back(tmp);
}

//双参 / 三参
void form_cmd_3(int num, char read[], int& count)
{
	tmp_ins tmp;
	tmp.No = num;
	char str[500];
	gt(str, read, count);
	int tmp_reg = reg[str];

	gt(str, read, count);
	map<string, int>::iterator reg_itr;
	reg_itr = reg.find(str);
	int a = 0;    //字符数组的访问位置
	if (reg_itr == reg.end()) {
		tmp.x = to_int(str, a);
		//tmp.x_exist = true;
		tmp.no_of_reg[1] = tmp_reg;
	}
	else {
		char str2[500];
		gt(str2, read, count);
		if (str2[0] == NULL || str2[0] == '#') {
			tmp.no_of_reg[1] = tmp_reg;
			tmp.no_of_reg[2] = reg[str];
		}
		else {
			tmp.no_of_reg[0] = tmp_reg;
			tmp.no_of_reg[1] = reg[str];

			map<string, int>::iterator reg_itr;
			reg_itr = reg.find(str2);
			int a = 0;   //字符数组的访问位置
			if (reg_itr == reg.end()) {
				tmp.x = to_int(str2, a);
				//tmp.x_exist = true;
			}
			else tmp.no_of_reg[2] = reg[str2];
		}
	}
	tmp_txt.push_back(tmp);
}

//从此处检查
//一个label / 寄存
void form_cmd_4(int num, char read[], int& count)
{
	tmp_ins tmp;
	tmp.No = num;
	char str[500];

	gt(str, read, count);
	map<string, int>::iterator reg_itr;
	reg_itr = reg.find(str);
	if (reg_itr == reg.end()) tmp.label = str;
	else tmp.no_of_reg[1] = reg[str];

	tmp_txt.push_back(tmp);
}

//1个label + 1个scr  + 1 个寄存(都不要存在第一个寄存器里)
void form_cmd_5(int num, char read[], int& count)
{
	tmp_ins tmp;
	tmp.No = num;
	char str[500];
	gt(str, read, count);
	tmp.no_of_reg[1] = reg[str];

	gt(str, read, count);
	map<string, int>::iterator reg_itr;
	reg_itr = reg.find(str);
	int a = 0;   //字符数组的访问位置
	if (reg_itr == reg.end()) {
		tmp.x = to_int(str, a);
		//tmp.x_exist = true;
	}
	else tmp.no_of_reg[2] = reg[str];

	gt(str, read, count);
	tmp.label = str;
	tmp_txt.push_back(tmp);
}

//1个label + 1个rscr
void form_cmd_6(int num, char read[], int& count)
{
	tmp_ins tmp;
	tmp.No = num;
	char str[500];
	gt(str, read, count);
	tmp.no_of_reg[1] = reg[str];

	gt(str, read, count);
	tmp.label = str;
	tmp_txt.push_back(tmp);
}

//load: 一个放一号寄存器， 一个address

//注意load可能的两种操作，看label和内存地址来定。
void form_cmd_7(int num, char read[], int& count)
{
	tmp_ins tmp;
	tmp.No = num;
	char str[500];
	gt(str, read, count);
	tmp.no_of_reg[0] = reg[str];

	gt(str, read, count);

	//while (read[count] == ' '|| read[count] == '\t' || read[count] == ',') ++count;
	if ((str[0] <= '9' && str[0] >= '0') || str[0] == '-') {
		int a = 0;
		tmp.x = to_int(str, a);
		//tmp.x_exist = true;
		gt(str, str, a);
		tmp.no_of_reg[1] = reg[str];
	}
	else {
		tmp.label = str;
	}
	tmp_txt.push_back(tmp);
}

//store: 1个非1寄存器， 一个address
void form_cmd_8(int num, char read[], int& count)
{
	tmp_ins tmp;
	tmp.No = num;
	char str[500];
	gt(str, read, count);
	tmp.no_of_reg[1] = reg[str];

	gt(str, read, count);

	//while (read[count] == ' '|| read[count] == '\t' || read[count] == ',') ++count;
	if ((str[0] <= '9' && str[0] >= '0') || str[0] == '-') {
		int a = 0;
		tmp.x = to_int(str, a);
		//tmp.x_exist = true;
		gt(str, str, a);
		tmp.no_of_reg[2] = reg[str];
	}
	else {
		tmp.label = str;
	}
	tmp_txt.push_back(tmp);
}

//一个放一号寄存器，还有一个未知
void form_cmd_9(int num, char read[], int& count)
{
	tmp_ins tmp;
	tmp.No = num;
	char str[500];
	gt(str, read, count);
	tmp.no_of_reg[0] = reg[str];

	gt(str, read, count);
	if (str[0] != NULL && str[0] != '#') {
		map<string, int>::iterator reg_itr;
		reg_itr = reg.find(str);
		int a = 0;   //字符数组的访问位置
		if (reg_itr == reg.end()) {
			tmp.x = to_int(str, a);
			//tmp.x_exist = true;
		}
		else tmp.no_of_reg[1] = reg[str];
	}

	tmp_txt.push_back(tmp);
}
