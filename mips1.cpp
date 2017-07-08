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

//����
//ofstream fout("log.out");
//�˴����ڴ��С�Ķ���
const int M = 1024 * 1024;
const int SIZE = 4 * M; //���Խ�����Ļ�
//const int SIZE = 10000000;
//���Ǵ��ļ�����ʱһ���ܷ�����������
const int MAX_SIZE = 5e3;

//�˴�Ϊ�Ĵ�����ע�⻹��������λ����λ����ļĴ���
int rgstr[34] = { 0 };   //ָ��32�żĴ���Ϊ��λ�Ĵ�����33�żĴ���Ϊ��λ�Ĵ���

						 //�˴�Ϊ�ڴ�
char Memory[SIZE + 1] = { 0 };  //����������Ƿ�ĳ�unsigned
int fp = 0; int sp = SIZE;      //fp����ѿռ��ָ֡�룬��ַ�ɵ͵��ߣ� sp����ջ�ռ�ĵ�ջָ�룬��ַ�ɸߵ��͡�
								//ע��fpָ��ָ����ǿյ��ڴ�
								//�˴���ָ������
map<string, int> label_local;
struct tmp_ins {
	int No = 0;
	short no_of_reg[3] = { 0 };
	int x = 0;    //bool x_exist = false;  //���ڿ�x�Ƿ����
	string label;                        //label����ת��Ϣ��Ҫ��һ���ӹ�

										 //�ڴ˴����˳�ֵ
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
	int No;                 //��������,���а�λ������ѹ��ָ����Ĳ���������ǧλ���������ĺ������
							//**1�żĴ���ר��д��ļĴ���
	short no_of_reg[3];
	//�������õ��ļĴ����ı��(����һ���Ĵ�����lw�Ȳ�����ʱ����ܻ����ԭʼ�����ݵ�ַ�����û��������ζ������label�������λ�÷��ʣ�
	int x;      //bool x_exist;               //����������ƫ������lw�Ȳ�����
	int label;                    //label����ת���,�漰ָ�����ת�;�̬�����ķ���

	instruction() {
		//x_exist = false;
	}
	instruction(const tmp_ins& obj) {
		No = obj.No;
		for (int i = 0; i < 3; ++i) no_of_reg[i] = obj.no_of_reg[i];
		x = obj.x;
		if (obj.label.length() == 0) label = -1;              //ע��labelΪ��Ҳ���ĳ���-1
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

//���ָ��ĵ�ַ
deque<tmp_ins> tmp_txt;
//deque<tmp_ins> text;
deque<instruction> text;
int main_start = 0;   //����ָ�ʼ�ĵط�

					  //����ӳ������Ĵ洢�ط�
map<string, int> opr;
map<string, int> reg;
map<string, int> data_opr;
map<int, string> map_out;

//�����￪ʼ�����ݶ��벿��
//������������Ԥ������Ҫ���������������    
void initialize();  //��ʼ��map�����ȡ�
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

//����ģ��CPU������ʼ�ĵط�
//����һ��ͳһ�Ĵ���弶��ˮ�ݴ���Ϣ�ĵط���ͬʱ����һ����պ���
//ͬʱ�һ���Ҫһ������������ʵ�ȫ�ֲ�������ģ��PC�Ĵ���
class performer {
private:
	struct product {
		int instruct_address;                      //�������ı��е�λ�ã�jal�ȿ��ܻ�ʹ��
												   //instruction �� label �� no_of_reg ���û�У���ֵΪ-1
		instruction instruct;                      //�����ֻ�ᱻ��ȡ�����ᱻ�޸�(����ܹ��ĳ��������)
		int data_from_reg[3] = { 0 };              //���ܻ���Ҫ��ȡ���������Ĵ��������ݣ�load���//�ڶ��׶��޸ģ������׶�ʹ��//�����������Ĵ���������ݣ�Ҳ������һ��ƫ���� / ��������һ���Ĵ�������
		int reg_to_be_input[2] = { -1, -1 };             //��Ҫ������ļĴ����ı��
		int ans_of_stage_3[2] = { 0, 0 };                 //�����������ݴ�, ������һ������������������һ���ڴ��ַ��������������(��32λ�͸�32λ)
		int data_from_Memory = 0;                  //�ӵ��Ĳ��ж�����������
												   //label ��һֱ���������������Ҫcpu��ִ��ʱ����洢
												   //ע������������Ϣ�б�0�Ķ�����������޷�ȷ������
		void fresh() {
			for (int i = 0; i <= 1; ++i) {
				reg_to_be_input[i] = -1;
				ans_of_stage_3[i] = 0;
			}
		}
	};

	product order[5];    //ǰ���׶���ÿ���׶�ִ����֮��Ľ���ݴ档ÿһ��ִ�У���һ��ʹ��ǰһ�����ݴ���

						 //������һ��Ҫ��ȡ������
						 //PC�Ĵ�����������ȷ����һ�׶��Ƿ���Է��ʣ���-1��Ϊ���ɷ��ʣ�
	int PC_order = -1;

	//����Ϊ����hazard����Ҫ��һЩ����
	int len;   //��ֹ����Խ��
	//�˱������ڿ��Ƴ����Ƿ��Ѿ�������
	bool finish = false;
	//�˱������ڵ�ǰ�׶��Ƿ���control hazard
	short control_hazard = 0;
	//�����жϼĴ����Ƿ����ڱ�ʹ��	
	short use_reg[34] = { 0 };
	//�˱������ڵ�ǰ�׶��Ƿ���data hazard
	short data_hazard = 0;
	//���׶���Ϣ��档����Ϣ��������
	bool can_read[5] = { false };

	//һ����һЩ���ߺ�����
	//���ڵڶ��׶ε���ˮ��
	//void other_prepare(product& obj);
	int reg_prepare(product& obj);
	//���ڵ����׶�
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
	void neg(product& obj) { obj.ans_of_stage_3[0] = -obj.data_from_reg[0]; }     //neg��ȡ������
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
	void syscall(product& obj);   //ע�����ָ���п����޸�finish��ֵ��

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

	//ע�⣺����׶β�����Ҫ������һ���Ƿ��ܷ���
	void Write_Bk() {
		if (!finish && can_read[4]) {
			//д�ز���
			//д��ǵ�����use_reg
			order[4] = order[3];

			int num = order[4].instruct.No % 100;
			if (23 <= num && num <= 39) {   //��תָ��
				--control_hazard;
				if (order[4].ans_of_stage_3[0]) {
					if (order[4].instruct.label != -1) PC_order = order[4].instruct.label;//ȷ����ʱ��һ�׶λ�û�б�syscall�޸�Ϊ���ɷ���
					else PC_order = order[4].data_from_reg[0];

					if (38 <= num && num <= 39) {  //�����jal/jalr
						rgstr[order[4].reg_to_be_input[0]] = order[4].instruct_address + 1;
						--use_reg[order[4].reg_to_be_input[0]];
					}
				}
			}
			else if (40 <= num && num <= 43) {  //load ����
				rgstr[order[4].reg_to_be_input[0]] = order[4].data_from_Memory;
				--use_reg[order[4].reg_to_be_input[0]];
			}
			else {
				if (num == 51) {           //syscall���ܽ�����������
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
			if (40 <= num && num <= 43) {    //load����
				if (num == 40) order[3].data_from_Memory = order[3].ans_of_stage_3[0];
				else if (num == 41) order[3].data_from_Memory = Memory[order[3].ans_of_stage_3[0]];
				else if (num == 42) order[3].data_from_Memory = *((short*)(Memory + order[3].ans_of_stage_3[0]));
				else if (num == 43) order[3].data_from_Memory = *((int*)(Memory + order[3].ans_of_stage_3[0]));

			}
			else if (44 <= num && num <= 46) {  //store
				if (num == 44) Memory[order[3].ans_of_stage_3[0]] = order[3].data_from_reg[0];
				else if (num == 45) new (static_cast<void*> (&Memory[order[3].ans_of_stage_3[0]])) short(order[3].data_from_reg[0]);  //�����ڴ�
				else if (num == 46) new (static_cast<void*> (&Memory[order[3].ans_of_stage_3[0]])) int(order[3].data_from_reg[0]);    //�����ڴ�
			}
			else if (num == 51) {
				if (order[3].ans_of_stage_3[1] == -10) {
					char tmp[2000];
					cin >> tmp;
					int len = strlen(tmp);
					int address = order[3].data_from_reg[0];
					for (int i = 0; i <= len; ++i) {
						new (static_cast<void*> (&Memory[address++])) char(tmp[i]);    //�����ڴ���
					}
				}
			}
			can_read[4] = true;
		}
		else can_read[4] = false;
		//load: ���ڴ��ַ������ȡ��
		//store : ������д���ڴ��У����ʱ���������Ѿ��ӼĴ����ж��˳���
	}

	void Execution() {
		if (!finish && can_read[2]) {
			order[2] = order[1];
			int num = order[2].instruct.No % 100;
			(this->*excute[num])(order[2]);//����ʹ�ú���ָ�����ʽ

			can_read[3] = true;
		}
		else can_read[3] = false;
	}

	void Decode_Pre() {
		if (!finish) {
			if (!data_hazard && !control_hazard && can_read[1]) {
				order[1] = order[0];

				//other_prepare(order[1]);   //��syscall�������ڴ˽׶����ȷ����Щ�Ĵ���Ҫ��д��

				//0���ǿ����Ҷ�ȡʧ�ܣ� 1�����ǿ��������ֶ�ȡʧ�ܣ� 2�������Ҷ�ȡ�ɹ���3���ǿ����Ҷ�ȡ�ɹ���
				int stage = reg_prepare(order[1]);         //���ں�������

				if (stage == 0) {            //��ȡʧ��
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
				//�����Ƿ�control_hazard���������ݶ��������ϵ��ˣ�can_read[1]������Ϊ1�ˣ�
				if (data_hazard) {
					//���ʼĴ����ɹ�
					if (reg_prepare(order[1]) >= 2) {
						--data_hazard;
						can_read[2] = true;
					}
					else can_read[2] = false;
				}
				else can_read[2] = false;  //�Ծɴ��ڲ��ɶ�ȡ�׶�
			}
		}
		else can_read[2] = false;
	}

	void Ins_Fetch() {
		//�����Ѿ�������������֮��ľͲ���load��hazard��Ҳ����Ҫ��
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

int performer::reg_prepare(performer::product& obj) {   //�ǵøĻ�����
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
	else if (23 <= num && num <= 39) {//��ת����
		for (int i = 1; i <= 2; ++i) {
			if (obj.instruct.no_of_reg[i] != -1 && use_reg[obj.instruct.no_of_reg[i]]) return 1;
		}
		//��Ϊ�����ݶ���
		for (int i = 1; i <= 2; ++i) {
			//use_reg[obj.instruct.no_of_reg[i]] = true;
			if (obj.instruct.no_of_reg[i] != -1) obj.data_from_reg[i - 1] = rgstr[obj.instruct.no_of_reg[i]];
		}
		//��Ϊ����д�ļĴ���׼����
		if (num == 38 || num == 39) {    //jal����
			obj.reg_to_be_input[0] = 31;
			++use_reg[obj.reg_to_be_input[0]];
		}
		/*else if (num == 39) {    //jalr����
		obj.reg_to_be_input[0] = 31;
		//obj.reg_to_be_input[1] = obj.instruct.no_of_reg[0]; //??������������ʲô�ģ�
		use_reg[obj.reg_to_be_input[0]] = true;
		//use_reg[obj.reg_to_be_input[1]] = true;
		}*/
		//��תϵ�������漸��������Ҫ��д��Ĵ�������
		/*else if (obj.instruct.no_of_reg[0] != -1) {
		obj.reg_to_be_input[0] = obj.instruct.no_of_reg[0];
		use_reg[obj.reg_to_be_input[0]] = true;
		}*/
		return 2;
	}
	else if (6 <= num && num <= 9) {                      //�˷����������
		for (int i = 1; i <= 2; ++i) {
			if (obj.instruct.no_of_reg[i] != -1 && use_reg[obj.instruct.no_of_reg[i]]) return 0;
		}
		//��Ϊ�����ݶ���
		for (int i = 1; i <= 2; ++i) {
			//use_reg[obj.instruct.no_of_reg[i]] = true;
			if (obj.instruct.no_of_reg[i] != -1) obj.data_from_reg[i - 1] = rgstr[obj.instruct.no_of_reg[i]];
		}
		//ȷ��д��ļĴ���
		if (obj.instruct.no_of_reg[0] == -1) {       //û��ָ��д��ļĴ���(0�żĴ�����������д���)
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
			//��������
			obj.data_from_reg[0] = rgstr[33];
			//׼��д��ļĴ���
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
		//��Ϊ�����ݶ���
		for (int i = 1; i <= 2; ++i) {
			//use_reg[obj.instruct.no_of_reg[i]] = true;
			if (obj.instruct.no_of_reg[i] != -1) obj.data_from_reg[i - 1] = rgstr[obj.instruct.no_of_reg[i]];
		}
		//ȷ��д��ļĴ���
		if (obj.instruct.no_of_reg[0] != -1) {
			obj.reg_to_be_input[0] = obj.instruct.no_of_reg[0];
			++use_reg[obj.reg_to_be_input[0]];
		}
		return 3;
	}
}

//������ǣ� ������/ƫ���� + Ҫд��ļĴ���
//syscall�ĵ�����ʵ��
void performer::syscall(performer::product& obj) {  //�ǵøĻ�����
	int loca;
	switch (obj.data_from_reg[2])
	{
	case 1:cout << obj.data_from_reg[0]; break;   //�ȼӻ��з����������
	case 4:
		loca = obj.data_from_reg[0];
		while (Memory[loca] != '\0') {
			cout << Memory[loca++];
		}
		break;
	case 5:cin >> obj.ans_of_stage_3[0]; break;
	case 8:
		obj.ans_of_stage_3[1] = -10; break;//ȡһ������ֵ��ʹ���ڵ�4�׶�ִ���ڴ����
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

//�������ģ�����
void Performance();

int main(int argc, char** argv)
{
	//ofstream out("log.txt");

	Input(argv[1]);

	rgstr[reg["$sp"]] = sp;                     //�ھ�̬������ֵ�����Ժ����Ͻ�ջ�ռ�����һ����ֵ����������ˮ�߿�ʼ֮ǰ
	main_start = label_local["main"];   //ȷ������ָ�ʼ�ĵط���

	Performance();

	//out.close();
	//system("pause");
	return 0;
}

//��ɻ�����map����
//�����漰������弶�ܹ�������
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

//������������Ԥ������Ҫ���������������
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
	bool deal_flag = true;  //false ��ʾ���ڴ���data�������ݣ� true ��ʾ���ڴ���text��������
	int loca = 0;           //����ͳ�Ƶ�ǰ��������

	map<string, int>::iterator opr_itr;
	while (fin.getline(tmp, MAX_SIZE)) {
		int count = 0;
		int len = strlen(tmp);       //len��ʾtmp���ַ�������
		while (count < len) {
			char str[500];           //����һ�����������������label���ַ���

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
					//ע��ʹ�õ����� -�����������Ը����Ӧ�ĺ���������
					form_instruction(str, loca, tmp, count); break;
					//������������γ��Ժ���һ��һ��û�ж�����
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
		//���Դ���
		/*CPU.Ins_Fetch();
		CPU.Decode_Pre();
		CPU.Execution();
		CPU.Memory_Access();
		CPU.Write_Bk();*/
	}
}

//ע��gt���������ո�ģ���ֻ������Ժ���һ���������ʱ�򣡼������gt����
//һ������gt��������ζ�����������֮ǰ����Ϣ�����㶼�Ѿ�û��������
//ɶ����û������strһ��NULL
void gt(char str[], char tmp[], int & count)
{
	int i = 0; int len = strlen(tmp);
	while ((tmp[count] == ' ' || tmp[count] == '\t' || tmp[count] == ',' || tmp[count] == '\0' || tmp[count] == '\"' || tmp[count] == '(') && count < len) ++count;

	bool use_flag = false;
	//���Կ��ǸĽ����ڡ�#��������
	while (tmp[count] != ' ' && tmp[count] != '\t' && tmp[count] != ',' && tmp[count] != ':' && tmp[count] != ')' && tmp[count] != '\0' && tmp[count] != '"' && count < len) {
		use_flag = true;
		str[i++] = tmp[count++];
	}
	str[i] = NULL;
	/*while (tmp[count] == '\0' || tmp[count] == ' ' || tmp[count] == '\t' )
	if (tmp[count] == ':' || tmp[count] == ')' || tmp[count] == '"') ++count;*/
}

//to_int Ҳ�����ո�����ע�����ʱ��ر�֤����������
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
			new (static_cast<void*> (&Memory[fp])) int(num);  //�����ڴ�
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
			new (static_cast<void*> (&Memory[fp])) int(num);  //�����ڴ�
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
			new (static_cast<void*> (&Memory[fp])) int(num);  //�����ڴ�
			fp += 4;
		}
	} while (str[0] != NULL && str[0] != '#');
}

void space_(char tmp[], int & count, bool & deal_flag) { fp += to_int(tmp, count); }
void data_(char tmp[], int & count, bool & deal_flag) { deal_flag = false; }
void text_(char tmp[], int & count, bool & deal_flag) { deal_flag = true; }

//form_label��������Ϊ�Լ���ָ������ϡ���ƨ�ɡ�������
void form_label(char label[], int loca, bool flag, char tmp[], int& count)
{
	if (!flag) label_local[label] = fp;  //��ʱ������Ǿ�̬����
	else label_local[label] = loca;      //�˴���ָ�����ת���ĵ�ַ

	int len = strlen(tmp);
	while ((tmp[count] == ' ' || tmp[count] == '\t' || tmp[count] == '\0') && count < len) ++count;
	if (tmp[count] == ':') ++count;
}

void form_instruction(char command[], int& loca, char tmp[], int& count)
{
	int num = opr[command];   //�Դ���һ��˼·�򵼡�
	form_cmd[(num / 10000) % 10](num, tmp, count);
	++loca;
}

//���һ�¼Ĵ������ź���û��
//�޲�����
void form_cmd_0(int num, char read[], int& count)
{
	tmp_ins tmp;
	tmp.No = num;
	tmp_txt.push_back(tmp);
}

//�����Ĵ��� || һ���Ĵ��� + ������
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

//�����Ĵ��� + һ���Ĵ� / ������
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
	int a = 0;   //�ַ�����ķ���λ��
	if (reg_itr == reg.end()) {
		tmp.x = to_int(str, a);
		//tmp.x_exist = true;
	}
	else tmp.no_of_reg[2] = reg[str];
	tmp_txt.push_back(tmp);
}

//˫�� / ����
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
	int a = 0;    //�ַ�����ķ���λ��
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
			int a = 0;   //�ַ�����ķ���λ��
			if (reg_itr == reg.end()) {
				tmp.x = to_int(str2, a);
				//tmp.x_exist = true;
			}
			else tmp.no_of_reg[2] = reg[str2];
		}
	}
	tmp_txt.push_back(tmp);
}

//�Ӵ˴����
//һ��label / �Ĵ�
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

//1��label + 1��scr  + 1 ���Ĵ�(����Ҫ���ڵ�һ���Ĵ�����)
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
	int a = 0;   //�ַ�����ķ���λ��
	if (reg_itr == reg.end()) {
		tmp.x = to_int(str, a);
		//tmp.x_exist = true;
	}
	else tmp.no_of_reg[2] = reg[str];

	gt(str, read, count);
	tmp.label = str;
	tmp_txt.push_back(tmp);
}

//1��label + 1��rscr
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

//load: һ����һ�żĴ����� һ��address

//ע��load���ܵ����ֲ�������label���ڴ��ַ������
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

//store: 1����1�Ĵ����� һ��address
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

//һ����һ�żĴ���������һ��δ֪
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
		int a = 0;   //�ַ�����ķ���λ��
		if (reg_itr == reg.end()) {
			tmp.x = to_int(str, a);
			//tmp.x_exist = true;
		}
		else tmp.no_of_reg[1] = reg[str];
	}

	tmp_txt.push_back(tmp);
}
