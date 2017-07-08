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
//const int SIZE = 4 * M; ���Խ�����Ļ�
const int SIZE = 4 * M;
//���Ǵ��ļ�����ʱһ���ܷ�����������
const int MAX_SIZE = 5e3;

//�˴�Ϊ�Ĵ�����ע�⻹��������λ����λ����ļĴ���
int rgstr[34] = { 0 };   //ָ��32�żĴ���Ϊ��λ�Ĵ�����33�żĴ���Ϊ��λ�Ĵ���

//�˴�Ϊ�ڴ�
char Memory[SIZE + 1] = { 0 };  //����������Ƿ�ĳ�unsigned
int fp = 0; int sp = SIZE;      //fp����ѿռ��ָ֡�룬��ַ�ɵ͵��ߣ� sp����ջ�ռ�ĵ�ջָ�룬��ַ�ɸߵ��͡�
                                //ע��fpָ��ָ����ǿյ��ڴ�

//���������ڷ�֧Ԥ���һЩ��Ϣ
//+1 mod 2 ��Ϊ��ȷ���
//+3 mod 2 ��Ϊ�������

const int N = 4;                //���洢������ε���ʷ��Ϣ
const int Amount = 16;          //�ܹ��м���ģʽ��Ҫʶ�� ��2^N��
struct branch {
	short history = 0;                    //�洢����Ĵεĸ÷�֧����ת���
	short Pattern[Amount] = { 2,2,2,2, 2,2,2,2, 2,2,2,2, 2,2,2,2 };        //ÿ��pattern�ı��ͼ�����
	//branch() { ; }
};

int predict_count = 0;
int predict_right = 0;

vector<branch> Store;
map<int, int> History_Table;

//��������Ϣ
map<int, string> map_out;
//�˴���ָ������
map<string, int> label_local;
struct tmp_ins{               
	int No = 0; 
	short no_of_reg[3] = { 0 };
	int x = 0;                           //���ڿ�x�Ƿ����
	string label;                        //label����ת��Ϣ��Ҫ��һ���ӹ�
	int address;

	//�ڴ˴����˳�ֵ
	tmp_ins() {
		for (int i = 0; i < 3; ++i) no_of_reg[i] = -1;
	}
	friend ostream& operator <<(ostream& os, const tmp_ins& obj) {
		os << map_out[obj.No % 100] << "\t";
		for (int i = 0; i < 3; ++i) {
			os << "reg " << i << " : " << obj.no_of_reg[i] << "\t";
		}
		os << "x: " << obj.x << "\t";
		os << "label: " << obj.label << "\t";
		return os;
	}
};

struct instruction {
	int No;                 //��������,���а�λ������ѹ��ָ����Ĳ���������ǧλ���������ĺ������
	//0�żĴ���ר��д��ļĴ���
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
		else {
			label = label_local.find(obj.label)->second;
			int num = No % 100;
			if (24 <= num && num <= 35) {
				Store.push_back(branch());
				History_Table[obj.address] = Store.size() - 1;   // ������ĵ�ַӳ�䵽�洢��λ��
			}
		}
	}

	friend ostream& operator <<(ostream& os, const instruction& obj) {
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

//�����￪ʼ�����ݶ��벿��
//������������Ԥ������Ҫ���������������    
	void initialize();  //��ʼ��map�����ȡ�
		void gt(char str[], char tmp[], int& count);
		int to_int(char tmp[], int& count);
		
		void align_(char tmp[], int & count, bool & deal_flag);
		void ascii_(char tmp[], int & count, bool & deal_flag);
		void asciiz_(char tmp[], int & count, bool & deal_flag);

		void alloc(char tmp[], int & count, int len);

		void byte_(char tmp[], int & count, bool & deal_flag);
		void half_(char tmp[], int & count, bool & deal_flag);
		void word_(char tmp[], int & count, bool & deal_flag);
		void space_(char tmp[], int & count, bool & deal_flag);
		void data_(char tmp[], int & count, bool & deal_flag);
		void text_(char tmp[], int & count, bool & deal_flag);
		
	void(*deal_with_data[9])(char[], int &, bool &) = { align_, ascii_, asciiz_, byte_, half_, word_, space_, data_, text_};

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
		instruction instruct;                      //��������ڵڶ���֮��Ӧ�þͲ����ٱ�����
		int num;                                   //������
		int reg_to_be_input[2] = { -1, -1 };       //������ļĴ�����
		int data_of_stage2[3] = { 0 };             //�ӵڶ�����ȡ�����κ���Ϣ�������ټӣ�
		int ans_of_stage3[2] = { 0 };			   //�������Ľ��
		int data_of_stage4 = 0;                    //�ӼĴ����������������
		bool predict_result = false;               //�ڷ�֧Ԥ��ʱ����֧Ԥ���������ǲ�������¼����
		int PC_buffer = 0;                         //��֧Ԥ��ʱ�洢ԭ�ȱ���Ҫ��ת������

		void fresh() {
			for (int i = 0; i <= 1; ++i) {
				reg_to_be_input[i] = -1;            //�����ټ�
				ans_of_stage3[i] = 0;
				data_of_stage2[i] = 0;
			}
			data_of_stage2[2] = 0;
		}
	};

	product order[5];    //ǰ���׶���ÿ���׶�ִ����֮��Ľ���ݴ档ÿһ��ִ�У���һ��ʹ��ǰһ�����ݴ���

	//������һ��Ҫ��ȡ������
	//PC�Ĵ�����������ȷ����һ�׶��Ƿ���Է��ʣ���-1��Ϊ���ɷ��ʣ�
	int PC_order = -1;

	//����Ϊ����hazard����Ҫ��һЩ����

	//�˱������ڿ��Ƴ����Ƿ��Ѿ�������
	bool finish = false;
	//�˱������ڵ�ǰ�׶��Ƿ���control hazard
	short ctrl_stop = 0;
	//�����жϼĴ����Ƿ����ڱ�ʹ��	
	short use_reg[34] = { 0 };
	//�˱������ڵ�ǰ�׶��Ƿ���data hazard
	short data_hazard = 0;
	//���׶���Ϣ��档����Ϣ��������
	bool can_read[5] = { false };

	//����������ʵ�ַ�֧Ԥ�����Ϣ
	
	//int PC_buffer = 0;
	inline bool predict(product& obj);
	int fix(product& obj);

	//һ����һЩ���ߺ�����
	//���ڵڶ��׶ε���ˮ��
	//void other_prepare(product& obj);
	//int reg_prepare(product& obj);
	int decode_0(performer::product& obj);
	int decode_1(performer::product& obj);
	int decode_2(performer::product& obj);
	int decode_3(performer::product& obj);
	int decode_4(performer::product& obj);
	int decode_5(performer::product& obj);
	int decode_6(performer::product& obj);
	int decode_7(performer::product& obj);
	int decode_8(performer::product& obj);
	int decode_9(performer::product& obj);
	int(performer::*decode[10]) (product&) = { &performer::decode_0, &performer::decode_1, &performer::decode_2,
		&performer::decode_3, &performer::decode_4, &performer::decode_5, &performer::decode_6, &performer::decode_7,
		&performer::decode_8, &performer::decode_9 };

	//���ڵ����׶�
	void add(product& obj) { obj.ans_of_stage3[0] = obj.data_of_stage2[0] + obj.data_of_stage2[1]; }
	void addu(product& obj) { obj.ans_of_stage3[0] = unsigned(obj.data_of_stage2[0]) + unsigned(obj.data_of_stage2[1]); }
	void addiu(product& obj) { add(obj); }
	void sub(product& obj) { obj.ans_of_stage3[0] = obj.data_of_stage2[0] - obj.data_of_stage2[1]; }
	void subu(product& obj) { obj.ans_of_stage3[0] = unsigned(obj.data_of_stage2[0]) - unsigned(obj.data_of_stage2[1]); }
	void mul(product& obj) {
		if (obj.instruct.no_of_reg[0] != -1) { obj.ans_of_stage3[0] = obj.data_of_stage2[0] * obj.data_of_stage2[1]; }
		else {
			long long tmp = obj.data_of_stage2[0] * obj.data_of_stage2[1];
			obj.ans_of_stage3[0] = int(tmp);
			obj.ans_of_stage3[1] = int(tmp >> 32);
		}
	}
	void mulu(product& obj) {
		if (obj.instruct.no_of_reg[0] != -1) { obj.ans_of_stage3[0] = unsigned(obj.data_of_stage2[0]) * unsigned(obj.data_of_stage2[1]); }
		else {
			long long tmp = unsigned(obj.data_of_stage2[0]) * unsigned(obj.data_of_stage2[1]);
			obj.ans_of_stage3[0] = int(tmp);
			obj.ans_of_stage3[1] = int(tmp >> 32);
		}
	}
	void div(product& obj) {
		if (obj.instruct.no_of_reg[0] != -1) { obj.ans_of_stage3[0] = obj.data_of_stage2[0] / obj.data_of_stage2[1]; }
		else {
			obj.ans_of_stage3[0] = obj.data_of_stage2[0] / obj.data_of_stage2[1];
			obj.ans_of_stage3[1] = obj.data_of_stage2[0] % obj.data_of_stage2[1];
		}
	}
	void divu(product& obj) {
		if (obj.instruct.no_of_reg[0] != -1) { obj.ans_of_stage3[0] = unsigned(obj.data_of_stage2[0]) / unsigned(obj.data_of_stage2[1]); }
		else {
			obj.ans_of_stage3[0] = unsigned(obj.data_of_stage2[0]) / unsigned(obj.data_of_stage2[1]);
			obj.ans_of_stage3[1] = unsigned(obj.data_of_stage2[0]) % unsigned(obj.data_of_stage2[1]);
		}
	}
	void xor_(product& obj) { obj.ans_of_stage3[0] = obj.data_of_stage2[0] ^ obj.data_of_stage2[1]; }
	void xoru(product& obj) { obj.ans_of_stage3[0] = unsigned(obj.data_of_stage2[0]) ^ unsigned(obj.data_of_stage2[1]); }
	void neg(product& obj) { obj.ans_of_stage3[0] = -obj.data_of_stage2[0]; }
	void negu(product& obj) { obj.ans_of_stage3[0] = unsigned(-obj.data_of_stage2[0]); }
	void rem(product& obj) { obj.ans_of_stage3[0] = obj.data_of_stage2[0] % obj.data_of_stage2[1]; }
	void remu(product& obj) { obj.ans_of_stage3[0] = unsigned(obj.data_of_stage2[0]) % unsigned(obj.data_of_stage2[1]); }
	void li(product& obj) { obj.ans_of_stage3[0] = obj.data_of_stage2[0]; }
	void seq(product& obj) { obj.ans_of_stage3[0] = (obj.data_of_stage2[0] == obj.data_of_stage2[1]); }
	void sge(product& obj) { obj.ans_of_stage3[0] = (obj.data_of_stage2[0] >= obj.data_of_stage2[1]); }
	void sgt(product& obj) { obj.ans_of_stage3[0] = (obj.data_of_stage2[0] > obj.data_of_stage2[1]); }
	void sle(product& obj) { obj.ans_of_stage3[0] = (obj.data_of_stage2[0] <= obj.data_of_stage2[1]); }
	void slt(product& obj) { obj.ans_of_stage3[0] = (obj.data_of_stage2[0] < obj.data_of_stage2[1]); }
	void sne(product& obj) { obj.ans_of_stage3[0] = (obj.data_of_stage2[0] != obj.data_of_stage2[1]); }
	void b(product& obj) { obj.ans_of_stage3[0] = true; }
	void beq(product& obj) { seq(obj); }
	void bne(product& obj) { sne(obj); }
	void bge(product& obj) { sge(obj); }
	void ble(product& obj) { sle(obj); }
	void bgt(product& obj) { sgt(obj); }
	void blt(product& obj) { slt(obj); }
	void beqz(product& obj) { obj.ans_of_stage3[0] = (obj.data_of_stage2[0] == 0); }
	void bnez(product& obj) { obj.ans_of_stage3[0] = (obj.data_of_stage2[0] != 0); }
	void blez(product& obj) { obj.ans_of_stage3[0] = (obj.data_of_stage2[0] <= 0); }
	void bgez(product& obj) { obj.ans_of_stage3[0] = (obj.data_of_stage2[0] >= 0); }
	void bgtz(product& obj) { obj.ans_of_stage3[0] = (obj.data_of_stage2[0] > 0); }
	void bltz(product& obj) { obj.ans_of_stage3[0] = (obj.data_of_stage2[0] < 0); }
	void j(product& obj) { obj.ans_of_stage3[0] = true; }
	void jr(product& obj) { obj.ans_of_stage3[0] = true; }
	void jal(product& obj) { obj.ans_of_stage3[0] = true; }
	void jalr(product& obj) { obj.ans_of_stage3[0] = true; }
	void la(product& obj) { obj.ans_of_stage3[0] = obj.data_of_stage2[0] + obj.data_of_stage2[1]; }
	void lb(product& obj) { la(obj); }
	void lh(product& obj) { la(obj); }
	void lw(product& obj) { la(obj); }
	void sb(product& obj) { obj.ans_of_stage3[0] = obj.data_of_stage2[0] + obj.data_of_stage2[1]; }
	void sh(product& obj) { sb(obj); }
	void sw(product& obj) { sb(obj); }
	void move(product& obj) { obj.ans_of_stage3[0] = obj.data_of_stage2[0]; }
	void mfhi(product& obj) { obj.ans_of_stage3[0] = obj.data_of_stage2[0]; }
	void mflo(product& obj) { obj.ans_of_stage3[0] = obj.data_of_stage2[0]; }
	void nop(product& obj) { ; }
	void syscall(product& obj);

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
	}
	bool end() { return finish; }

	//���� �� ��׶λ����ϲ�ʹ�ú���ָ����ʵ��
	//ע�⣺����׶β�����Ҫ������һ���Ƿ��ܷ���
	void Write_Bk() {
		if (!finish && can_read[4]) {
			//д�ز���
			//д��ǵ�����use_reg
			order[4] = order[3];

			int num = order[4].instruct.No % 100;
			//���������PC�Ĵ�������
			if (num == 37 || num == 39) {  //jr / jalr
				--ctrl_stop;
				PC_order = order[4].data_of_stage2[2];
			}


			if (38 <= num && num <= 39) {  //�����jal/jalr
				rgstr[order[4].reg_to_be_input[0]] = order[4].instruct_address + 1;
				--use_reg[order[4].reg_to_be_input[0]];
			}
			/*if (23 <= num && num <= 39) {   //��תָ��
			}
				if (order[4].ans_of_stage3[0]) {
					PC_order = order[4].data_of_stage2[2];
				}
			}*/
			else if (40 <= num && num <= 43) {  //load ����
				rgstr[order[4].reg_to_be_input[0]] = order[4].data_of_stage4;
				--use_reg[order[4].reg_to_be_input[0]];
			}
			else {
				if (num == 51) {           //syscall���ܽ�����������
					if (order[4].data_of_stage2[2] == 10 || order[4].data_of_stage2[2] == 17) {
						finish = true;
					}
				}
				for (int i = 0; i <= 1; ++i) {
					if (order[4].reg_to_be_input[i] != -1) {
						rgstr[order[4].reg_to_be_input[i]] = order[4].ans_of_stage3[i];
						--use_reg[order[4].reg_to_be_input[i]];
					}
				}
			}
		}
	}

	void Memory_Access() {
		if (!finish && can_read[3]) {
			order[3] = order[2];
			int no = order[2].num % 100;
			if (40 <= no && no <= 43) {    //load����
				if (no == 40) order[3].data_of_stage4 = order[3].ans_of_stage3[0];
				else if (no == 41) order[3].data_of_stage4 = Memory[order[3].ans_of_stage3[0]];
				else if (no == 42) order[3].data_of_stage4 = *((short*)(Memory + order[3].ans_of_stage3[0]));
				else if (no == 43) order[3].data_of_stage4 = *((int*)(Memory + order[3].ans_of_stage3[0]));

			}
			else if (44 <= no && no <= 46) {  //store
				if (no == 44) Memory[order[3].ans_of_stage3[0]] = order[3].data_of_stage2[2];
				else if (no == 45) new (static_cast<void*> (&Memory[order[3].ans_of_stage3[0]])) short(order[3].data_of_stage2[2]);  //�����ڴ�
				else if (no == 46) new (static_cast<void*> (&Memory[order[3].ans_of_stage3[0]])) int(order[3].data_of_stage2[2]);    //�����ڴ�
			}
			else if (no == 51) {
				if (order[3].data_of_stage2[2] == 8) {
					char tmp[2000];
					cin >> tmp;
					int len = strlen(tmp);
					int address = order[3].data_of_stage2[0];
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
			int no = order[2].num % 100;
			(this->*excute[no])(order[2]);//����ʹ�ú���ָ�����ʽ

			if (24 <= no && no <= 35) {
				int stage = fix(order[2]);     //fix����������Ԥ������ʵ�ʽ���������ͼ��������������Ƿ�Ԥ����ȷ
				/*0��Ԥ�⣺��ת    ʵ�ʣ���ת    ��Ӱ��
				  1��Ԥ�⣺��ת    ʵ�ʣ�����    ������ʱ���֣����֮ǰ���������ݣ����ҽ�PC�Ĵ�����ֵ�޸�Ϊ��ָ�����һ��ָ��ָ���
				  0��Ԥ�⣺����    ʵ�ʣ�����    ��Ӱ��
				  2��Ԥ�⣺����    ʵ�ʣ���ת    ������ʱ���֣����֮ǰ�Ĳ������ݣ���PC�Ĵ�����ֵ�޸�ΪҪ��ת��ֵ  
				*/
				if (stage == 1) {
					can_read[1] = false;
					PC_order = order[2].PC_buffer;
				}
				else if (stage == 2) {
					can_read[1] = false;
					PC_order = order[2].data_of_stage2[2];
				}
			}
			can_read[3] = true;
		}
		else can_read[3] = false;
	}

	//�����޸�decode֮��ı��룡
	void Decode_Pre() {
		if (!finish) {
			if (can_read[1] && !data_hazard && !ctrl_stop) {
				order[1] = order[0];
				//0���ǿ����Ҷ�ȡʧ�ܣ� 1���ǿ����Ҷ�ȡ�ɹ��� 2��jr,jalr:��ȡ�ɹ�
				//3��jr,jalr����ȡʧ��    1��b,j,jal (ֱ���޸�PC�Ĵ�������������data_harzard��
				//0���������������ת�����Ҷ�ȡʧ��   1���������������ת�����Ҷ�ȡ�ɹ�   //�����߶��������crtl stop
				int stage;
				if (order[1].num % 100 != 50) stage = (this->*decode[(order[0].num / 100) % 10])(order[1]);         //���ں�������
				else stage = 3;

				switch (stage) {
					//0���ǿ����Ҷ�ȡʧ�ܣ� 1�����ǿ��������ֶ�ȡʧ�ܣ� 2�������Ҷ�ȡ�ɹ���3���ǿ����Ҷ�ȡ�ɹ���
					case 0: ++data_hazard; can_read[2] = false; break;
					case 1: can_read[2] = true; break;
					case 2: ++ctrl_stop; can_read[2] = true; break;
					case 3: ++data_hazard; ++ctrl_stop; can_read[2] = false; break;
				}
			}
			else {
				/*����ͬʱ��Ϊ���ݷ��ʳ�ͻ��ͣ�͵�jal�����һ����data_hazard״̬�¼�⵽���ݿ��Է����ˣ���ͬʱҲ��ζ��
				  crtl_stop��״̬�Ѿ��������
				*/
				if (data_hazard) {
					//���ʼĴ����ɹ�
					int stage = (this->*decode[(order[0].num / 100) % 10])(order[1]);
					if (stage == 1 || stage == 2 ) {
						--data_hazard;
						can_read[2] = true;
					}
					else can_read[2] = false;
				}
				else can_read[2] = false;
			}
		}
		else can_read[2] = false;
	}

	void Ins_Fetch() {
		//�����Ѿ�������������֮��ľͲ���load��hazard��Ҳ����Ҫ��
		if (can_read[0] && !finish && !ctrl_stop && !data_hazard) {
			order[0].fresh();
			order[0].instruct = text[PC_order];
			order[0].instruct_address = PC_order;
			order[0].num = order[0].instruct.No;
			/*fout << PC_order << "\t" << map_out[order[0].instruct.No % 100] << "\t" << order[0].instruct << "\n";
			for (int i = 0; i <= 33; ++i) {
				fout << i << "\t";
			}
			fout << "\n";
			for (int i = 0; i <= 33; ++i) {
				fout << rgstr[i] << "\t";
			}
			//fout << "\n";
			//fout << "Memory 44: " << int(Memory[44]);
			fout << "\n" << "\n" << "\n";
			/*for (int i = 0; i <= 33; ++i) {
				if (use_reg[i] < 0) {
					finish = true;
					return;
				}
			}*/
			/*if (PC_order == 360) {
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

//����Ԥ����
inline bool performer::predict(performer::product& obj) {
	//instruct_address��������ӳ��

	++predict_count;

	branch& now = Store[History_Table[obj.instruct_address]];
	if (now.Pattern[now.history] >= 2) return true;   //����ǿ��������תʱѡ����ת
	else return false;
}

//�����޸�Ԥ��״̬֮��
int performer::fix(performer::product& obj) {
	branch& now = Store[History_Table[obj.instruct_address]];
	//�޸�Ԥ����
	if (obj.ans_of_stage3[0]) {
		if (now.Pattern[now.history] <= 3) ++now.Pattern[now.history];
	}
	else {
		if (now.Pattern[now.history] >= 0) --now.Pattern[now.history];
	}
	//�޸ĸ�������ʷ
	now.history = (now.history * 2 + obj.ans_of_stage3[0]) % 16;

	//������һԤ���������
	if (obj.ans_of_stage3[0] == obj.predict_result) {
		++predict_right;
		return 0;
	}
	else if (obj.predict_result && !obj.ans_of_stage3[0]) return 1;
	else if (obj.ans_of_stage3[0] && !obj.predict_result) return 2;
}


//����һ��д��Ĵ��� + һ������Ĵ��� + ����Ĵ��� / ������
int performer::decode_0(performer::product& obj) {
	//������Ĵ���ʹ�����
	for (int i = 1; i <= 2; ++i) {
		if (obj.instruct.no_of_reg[i] != -1) {
			if (use_reg[obj.instruct.no_of_reg[i]]) return 0;
		}
	}
	//��Ϊ�����ݶ���
	for (int i = 1; i <= 2; ++i) {
		//use_reg[obj.instruct.no_of_reg[i]] = true;
		if (obj.instruct.no_of_reg[i] != -1) obj.data_of_stage2[i - 1] = rgstr[obj.instruct.no_of_reg[i]];
		else obj.data_of_stage2[i - 1] = obj.instruct.x;
	}
	//ȷ��д��ļĴ���
	obj.reg_to_be_input[0] = obj.instruct.no_of_reg[0];
	++use_reg[obj.reg_to_be_input[0]];
	return 1;
}

//����div / mul��
int performer::decode_1(performer::product& obj) {
	//������Ĵ���ʹ�����
	for (int i = 1; i <= 2; ++i) {
		if (obj.instruct.no_of_reg[i] != -1) {
			if (use_reg[obj.instruct.no_of_reg[i]]) return 0;
		}
	}
	//��Ϊ�����ݶ���
	for (int i = 1; i <= 2; ++i) {
		//use_reg[obj.instruct.no_of_reg[i]] = true;
		if (obj.instruct.no_of_reg[i] != -1) obj.data_of_stage2[i - 1] = rgstr[obj.instruct.no_of_reg[i]];
		else obj.data_of_stage2[i - 1] = obj.instruct.x;
	}

	//ȷ��д��Ĵ���
	if (obj.instruct.no_of_reg[0] == -1) {       //û��ָ��д��ļĴ���(0�żĴ�����������д���)
		obj.reg_to_be_input[0] = 32;             //32��low
		obj.reg_to_be_input[1] = 33;             //33��high
		++use_reg[32]; ++use_reg[33];
	}
	else {
		obj.reg_to_be_input[0] = obj.instruct.no_of_reg[0];
		++use_reg[obj.reg_to_be_input[0]];
	}
	return 1;
}

//һ��д��Ĵ��� + һ������Ĵ��� / ������(�ǵô���move)
int performer::decode_2(performer::product& obj) {
	//������Ĵ���
	if (obj.instruct.no_of_reg[1] != -1) {
		if (use_reg[obj.instruct.no_of_reg[1]]) return 0;
	}

	//�����ݶ���
	if (obj.instruct.no_of_reg[1] != -1) obj.data_of_stage2[0] = rgstr[obj.instruct.no_of_reg[1]];
	else obj.data_of_stage2[0] = obj.instruct.x;

	//ȷ��д��Ĵ���
	obj.reg_to_be_input[0] = obj.instruct.no_of_reg[0];
	++use_reg[obj.reg_to_be_input[0]];
	return 1;
}

//��j��ת����: һ������Ĵ��� + ����Ĵ��� / ������
int performer::decode_3(performer::product& obj) {
	//������Ĵ���
	for (int i = 1; i <= 2; ++i) {
		if (obj.instruct.no_of_reg[i] != -1) {
			if (use_reg[obj.instruct.no_of_reg[i]]) return 0;
		}
	}
	//��Ϊ�����ݶ���
	for (int i = 1; i <= 2; ++i) {
		if (obj.instruct.no_of_reg[i] != -1) obj.data_of_stage2[i - 1] = rgstr[obj.instruct.no_of_reg[i]];
		else obj.data_of_stage2[i - 1] = obj.instruct.x;
	}

	obj.data_of_stage2[2] = obj.instruct.label;
	
	obj.predict_result = predict(obj);
	if (obj.predict_result) {     //predict������Ԥ���Ƿ����ת
		obj.PC_buffer = PC_order;
		PC_order = obj.data_of_stage2[2];
	}
	return 1;
}

// ��j��ת����: һ������Ĵ���
int performer::decode_4(performer::product& obj) {
	if (use_reg[obj.instruct.no_of_reg[1]]) return 0;
	obj.data_of_stage2[0] = rgstr[obj.instruct.no_of_reg[1]];

	obj.data_of_stage2[2] = obj.instruct.label;
	obj.predict_result = predict(obj);
	if (obj.predict_result) {     //predict������Ԥ���Ƿ����ת
		obj.PC_buffer = PC_order;
		PC_order = obj.data_of_stage2[2];
	}
	return 1;
}

//b / j / jr / jal / jalr
int performer::decode_5(performer::product& obj) {
	if (obj.instruct.no_of_reg[1] != -1) {
		if (use_reg[obj.instruct.no_of_reg[1]]) return 3;  //ֻ�� jr �� jalr ���ȡʧ��
	}
	if (obj.instruct.no_of_reg[1] != -1) {
		obj.data_of_stage2[2] = rgstr[obj.instruct.no_of_reg[1]];
	}
	else obj.data_of_stage2[2] = obj.instruct.label;

	int ref = obj.num % 100;
	if (ref == 38 || ref == 39) {    //jal���� / jalr
		obj.reg_to_be_input[0] = 31;
		++use_reg[obj.reg_to_be_input[0]];
	}

	if (ref == 37 || ref == 39) {//jr, jalr
		return 2;
	}
	else {
		PC_order = obj.data_of_stage2[2];
		return 1;
	}
}

//mfhi / mflo
int performer::decode_6(performer::product& obj) {
	int use;
	if (obj.num % 100 == 48) use = 33;
	else if (obj.num % 100 == 49) use = 32;
	
	if (use_reg[use]) return 0;  //������Ĵ���ʹ�����
	else {
		//use_reg[33] = true;
		//��������
		obj.data_of_stage2[0] = rgstr[use];
		//׼��д��ļĴ���
		obj.reg_to_be_input[0] = obj.instruct.no_of_reg[0];
		++use_reg[obj.reg_to_be_input[0]];
		return 1;
	}
}

//load
int performer::decode_7(performer::product& obj) {
	if (obj.instruct.no_of_reg[1] != -1) {
		if (use_reg[obj.instruct.no_of_reg[1]]) return 0;   //���ݶ���
		else {  //׼������
			obj.data_of_stage2[0] = rgstr[obj.instruct.no_of_reg[1]];
			obj.data_of_stage2[1] = obj.instruct.x;
		}
	}
	else {  //׼������
		obj.data_of_stage2[0] = obj.instruct.label;
		obj.data_of_stage2[1] = 0;
	}

	//��Ҫд��ļĴ���
	obj.reg_to_be_input[0] = obj.instruct.no_of_reg[0];
	++use_reg[obj.reg_to_be_input[0]];
	return 1;
}

//store
int performer::decode_8(performer::product& obj) {
	//������Ĵ���ʹ�����
	for (int i = 1; i <= 2; ++i) {
		if (obj.instruct.no_of_reg[i] != -1) {
			if (use_reg[obj.instruct.no_of_reg[i]]) return 0;
		}
	}
	//��Ϊ�����ݶ���

	obj.data_of_stage2[2] = rgstr[obj.instruct.no_of_reg[1]];     //���Ż�������Ҫд���ڴ������
	if (obj.instruct.no_of_reg[2] != -1) {
		obj.data_of_stage2[0] = rgstr[obj.instruct.no_of_reg[2]];
		obj.data_of_stage2[1] = obj.instruct.x;
	}
	else {
		obj.data_of_stage2[0] = obj.instruct.label;
		obj.data_of_stage2[1] = 0;
	}
	return 1;
}
//syscall
//�涨�����������2�������ݴ���
int performer::decode_9(performer::product& obj) {
	if (use_reg[reg["$v0"]]) return 0;
	else {
		int choice = rgstr[reg["$v0"]];
		obj.data_of_stage2[2] = choice;       //syscall��ѡ��Ҳ�Ƿ���2�Ż�����
		if (choice == 1 || choice == 4) {
			if (use_reg[reg["$a0"]]) return 0;
			else {
				obj.data_of_stage2[0] = rgstr[reg["$a0"]];
				return 1;
			}
		}
		else if (choice == 5) {
			obj.reg_to_be_input[0] = reg["$v0"];
			++use_reg[reg["$v0"]];
			return 1;
		}
		else if (choice == 8) {
			if (use_reg[reg["$a0"]] || use_reg[reg["$a1"]]) return 0;
			else {
				obj.data_of_stage2[0] = rgstr[reg["$a0"]];
				obj.data_of_stage2[1] = rgstr[reg["$a1"]];
				return 1;
			}
		}
		else if (choice == 9) {
			if (use_reg[reg["$a0"]]) return 0;
			else {
				obj.data_of_stage2[0] = rgstr[reg["$a0"]];
				obj.reg_to_be_input[0] = reg["$v0"];

				++use_reg[reg["$v0"]];
				return 1;
			}
		}
		else if (choice == 10) {
			can_read[0] = false;
			return 1;
		}
		else if (choice == 17) {
			can_read[0] = false;
			if (use_reg[reg["$a0"]]) return 0;
			else {
				obj.data_of_stage2[0] = rgstr[reg["$a0"]];
				return 1;
			}
		}
	}
}


//������ǣ� ������/ƫ���� + Ҫд��ļĴ���
//syscall�ĵ�����ʵ��
void performer::syscall(performer::product& obj) {  //�ǵøĻ�����
	int loca; 
	switch (obj.data_of_stage2[2])
	{
	case 1:cout << obj.data_of_stage2[0]; break;   //�ȼӻ��з����������
	case 4:
		loca = obj.data_of_stage2[0];
		while (Memory[loca] != '\0') {
			cout << Memory[loca++];
		}
		break;
	case 5:cin >> obj.ans_of_stage3[0]; break;
	case 8:break;//ȡһ������ֵ��ʹ���ڵ�4�׶�ִ���ڴ���� break;//ȡһ������ֵ��ʹ���ڵ�4�׶�ִ���ڴ����
	case 9:
		if (fp % 4 != 0) {
			int k = fp / 4 + 1;
			fp = k * 4;
		}
		obj.ans_of_stage3[0] = fp;
		fp += obj.data_of_stage2[0];
		break;
	case 10: case 17: break;
	}
}

//�������ģ�����
void Performance();

int main(int argc, char** argv)
{
	Input(argv[1]);

	rgstr[reg["$sp"]] = sp;                     //�ھ�̬������ֵ�����Ժ����Ͻ�ջ�ռ�����һ����ֵ����������ˮ�߿�ʼ֮ǰ
	main_start = label_local["main"];   //ȷ������ָ�ʼ�ĵط���

	Performance();

	cerr << "right rate: " << double(predict_right) / double(predict_count) << "\n";

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

	opr["add"] = 21001; opr["addu"] = 21002;
	opr["addiu"] = 21003; opr["sub"] = 21004;
	opr["subu"] = 21005; opr["mul"] = 31106;
	opr["mulu"] = 31107; opr["div"] = 31108;
	opr["divu"] = 31109; opr["xor"] = 21010;
	opr["xoru"] = 21011; opr["neg"] = 11212;
	opr["negu"] = 11213; opr["rem"] = 21014;
	opr["remu"] = 21015;
	opr["li"] = 92216;
	opr["seq"] = 23017; opr["sge"] = 23018;
	opr["sgt"] = 23019; opr["sle"] = 23020;
	opr["slt"] = 23021; opr["sne"] = 23022;
	opr["b"] = 44523; opr["beq"] = 54324;
	opr["bne"] = 54325; opr["bge"] = 54326;
	opr["ble"] = 54327; opr["bgt"] = 54328;
	opr["blt"] = 54329; opr["beqz"] = 64430;
	opr["bnez"] = 64431; opr["blez"] = 64432;
	opr["bgez"] = 64433; opr["bgtz"] = 64434;
	opr["bltz"] = 64435; opr["j"] = 44536;
	opr["jr"] = 44537; opr["jal"] = 44538;
	opr["jalr"] = 44539;
	opr["la"] = 75740; opr["lb"] = 75741;
	opr["lh"] = 75742; opr["lw"] = 75743;
	opr["sb"] = 86844; opr["sh"] = 86845;
	opr["sw"] = 86846;
	opr["move"] = 97247; opr["mfhi"] = 97648;
	opr["mflo"] = 97649;
	opr["nop"] = 8050;
	opr["syscall"] = 9951;

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
	//ifstream fin("pi-5090379042-jiaxiao.s");
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
			if (count >= len) break;

			if (tmp[count] == '#') break;
			gt(str, tmp, count);
			if (str[0] == '.') {
				deal_with_data[data_opr[str]](tmp, count, deal_flag); break;
			}
			else{
				opr_itr = opr.find(str);
				if (opr_itr == opr.end()) {
					form_label(str, loca, deal_flag, tmp, count);
				}
				else {
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
	}
}

//ע��gt���������ո�ģ���ֻ������Ժ���һ���������ʱ�򣡼������gt����
//һ������gt��������ζ�����������֮ǰ����Ϣ�����㶼�Ѿ�û��������
//ɶ����û������strһ��NULL
void gt(char str[], char tmp[], int & count)
{
	int i = 0; int len = strlen(tmp);
	while ((tmp[count] == ' ' || tmp[count] == '\t' || tmp[count] == ',' || tmp[count] == '\0' || tmp[count] == '\"' || tmp[count] == '(' )&& count < len) ++count;
	
	bool use_flag = false;
	//���Կ��ǸĽ����ڡ�#��������
	while (tmp[count] != ' ' && tmp[count] != '\t' && tmp[count] != ',' && tmp[count] != ':' && tmp[count] != ')' && tmp[count] != '\0' && tmp[count] != '"' && count < len) {
		use_flag = true;
		str[i++] = tmp[count++];
	}
	str[i] = NULL;
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
	ascii_(tmp, count, deal_flag);
	Memory[fp++] = NULL;
}

void alloc(char tmp[], int & count, int len) {
	int num;
	char str[500];
	do {
		gt(str, tmp, count);
		if (str[0] != NULL && str[0] != '#') {
			int a = 0;
			num = to_int(str, a);
			new (static_cast<void*> (&Memory[fp])) int(num);  //�����ڴ�
			fp += len;
		}
	} while (str[0] != NULL && str[0] != '#');
}

void byte_(char tmp[], int & count, bool & deal_flag) { alloc(tmp, count, 1); }
void half_(char tmp[], int & count, bool & deal_flag) { alloc(tmp, count, 2); }
void word_(char tmp[], int & count, bool & deal_flag) { alloc(tmp, count, 4); }
void space_(char tmp[], int & count, bool & deal_flag) { fp += to_int(tmp, count); }
void data_(char tmp[], int & count, bool & deal_flag) { deal_flag = false;}
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
	tmp_txt[loca].address = loca;
	++loca;
}

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
	if (reg_itr == reg.end()) tmp.x = to_int(str, a);
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
			if (reg_itr == reg.end()) tmp.x = to_int(str2, a);
			else tmp.no_of_reg[2] = reg[str2];
		}
	}
	tmp_txt.push_back(tmp);
}

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
	if (reg_itr == reg.end()) tmp.x = to_int(str, a);
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
	if ((str[0] <= '9' && str[0] >= '0') || str[0] == '-') {
		int a = 0;
		tmp.x = to_int(str, a);
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
		if (reg_itr == reg.end()) tmp.x = to_int(str, a);
		else tmp.no_of_reg[1] = reg[str];
	}

	tmp_txt.push_back(tmp);
}
