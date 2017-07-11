#include <stdio.h>
#include <iostream>
#include <fstream>
#include <map>
#include <deque>
#include <string>
#include <cstring>
#include <cmath>
#include <queue>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>
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

//��������Ϣ
map<int, string> map_out;
//�˴���ָ������
map<string, int> label_local;
struct tmp_ins{               
	int No = 0; 
	short no_of_reg[3] = { 0 };
	int x = 0;                           //���ڿ�x�Ƿ����
	string label;                        //label����ת��Ϣ��Ҫ��һ���ӹ�

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
	int x;                  //����������ƫ������lw�Ȳ�����
	int label;              //label����ת���,�漰ָ�����ת�;�̬�����ķ���

	instruction() {}
	instruction(const tmp_ins& obj) {
		No = obj.No;
		for (int i = 0; i < 3; ++i) no_of_reg[i] = obj.no_of_reg[i];
		x = obj.x;
		if (obj.label.length() == 0) label = -1;              //ע��labelΪ��Ҳ���ĳ���-1
		else {
			label = label_local.find(obj.label)->second;
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

//�˴������ڶ��̵߳ļ�������
//std::mutex mtx; // ȫ�ֻ�����.
//std::condition_variable cv; // ȫ����������.
//atomic<bool> ready = false; // ȫ�ֱ�־λ.   //�ǵ�Ҫ������Ϊatomic

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

		void fresh() {
			for (int i = 0; i <= 1; ++i) {
				reg_to_be_input[i] = -1;            //�����ټ�
				ans_of_stage3[i] = 0;
				data_of_stage2[i] = 0;
			}
			data_of_stage2[2] = 0;
		}
	};

	product repository[6];    //5���ݴ�Ĳֿ�

	//������һ��Ҫ��ȡ������
	//PC�Ĵ�����������ȷ����һ�׶��Ƿ���Է��ʣ���-1��Ϊ���ɷ��ʣ�
	//next_PC_order ��һ���ǵ�һ�����ɵģ��ڶ����ǵ��岽���ɵġ�����е��岽���ɵ�Ӧ����ϸ����

	int PC_order = -1;
	int next_PC_order = -1;
	//int next_PC_order5 = -1;
	//����Ϊ����hazard����Ҫ��һЩ����

	//�˱������ڿ��Ƴ����Ƿ��Ѿ�������
	atomic<bool> finish;
	bool next_finish;
	//�˱������ڵ�ǰ�׶��Ƿ���control hazard
	short ctrl_stop = 0;
	short next_ctrl_stop = 0;
	//�����жϼĴ����Ƿ����ڱ�ʹ��	
	short use_reg[34] = { 0 };
	short next_use_reg[34] = { 0 };
	//�˱������ڵ�ǰ�׶��Ƿ���data hazard
	short data_hazard = 0;
	short next_data_hazard = 0;
	//���׶���Ϣ��档����Ϣ��������
	bool data_ready[6] = { 0 };
	bool next_data_ready[6] = { 0 };

	//�Ƿ���ת
	bool jump = false;

	//ÿ���̵߳ķ��ʱ��
	atomic<bool> ready[6];

	//һ����һЩ���ߺ�����
	//���ڵڶ��׶ε���ˮ��
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
		data_ready[1] = true;
		next_data_ready[1] = true;
		jump = false;
		finish = false;
		next_finish = false;
		for (int i = 1; i <= 5; ++i) {
			ready[i] = false;
		}
	}
	bool end() { return finish; }

	//���� �� ��׶λ����ϲ�ʹ�ú���ָ����ʵ��
	//ע�⣺����׶β�����Ҫ������һ���Ƿ��ܷ���
	void Write_Bk() {
		while (true) {
			//std::unique_lock <std::mutex> lck(mtx);
			//lck.unlock();
			while (!ready[5] && !finish) { ; }
			if (finish) break;
			//lck.lock();

			if (!finish && data_ready[5]) {
				//д�ز���
				//д��ǵ�����use_reg
				//repository[4] = repository[3];

				int num = repository[5].instruct.No % 100;
				if (23 <= num && num <= 39) {   //��תָ��
					next_ctrl_stop = ctrl_stop - 1;
					if (repository[5].ans_of_stage3[0]) {
						next_PC_order = repository[5].data_of_stage2[2];
						jump = true;

						if (38 <= num && num <= 39) {  //�����jal/jalr
							rgstr[repository[5].reg_to_be_input[0]] = repository[5].instruct_address + 1;
							next_use_reg[repository[5].reg_to_be_input[0]] = use_reg[repository[5].reg_to_be_input[0]] - 1;
						}
					}
				}
				else if (40 <= num && num <= 43) {  //load ����
					rgstr[repository[5].reg_to_be_input[0]] = repository[5].data_of_stage4;
					next_use_reg[repository[5].reg_to_be_input[0]] = use_reg[repository[5].reg_to_be_input[0]] - 1;
				}
				else {
					if (num == 51) {           //syscall���ܽ�����������
						if (repository[5].data_of_stage2[2] == 10 || repository[5].data_of_stage2[2] == 17) {
							next_finish = true;
						}
					}
					for (int i = 0; i <= 1; ++i) {
						if (repository[5].reg_to_be_input[i] != -1) {
							rgstr[repository[5].reg_to_be_input[i]] = repository[5].ans_of_stage3[i];
							next_use_reg[repository[5].reg_to_be_input[i]] = use_reg[repository[5].reg_to_be_input[i]] - 1;
						}
					}
				}
			}

			ready[5] = false;
		}
	}

	void Memory_Access() {

		while (true) {
			//std::unique_lock <std::mutex> lck(mtx);
			//lck.unlock();
			while (!ready[4] && !finish) { ; }
			if (finish) break;
			//lck.lock();

			if (!finish && data_ready[4]) {
				//repository[3] = repository[2];
				int no = repository[4].num % 100;
				if (40 <= no && no <= 43) {    //load����
					if (no == 40) repository[4].data_of_stage4 = repository[4].ans_of_stage3[0];
					else if (no == 41) repository[4].data_of_stage4 = Memory[repository[4].ans_of_stage3[0]];
					else if (no == 42) repository[4].data_of_stage4 = *((short*)(Memory + repository[4].ans_of_stage3[0]));
					else if (no == 43) repository[4].data_of_stage4 = *((int*)(Memory + repository[4].ans_of_stage3[0]));

				}
				else if (44 <= no && no <= 46) {  //store
					if (no == 44) Memory[repository[4].ans_of_stage3[0]] = repository[4].data_of_stage2[2];
					else if (no == 45) new (static_cast<void*> (&Memory[repository[4].ans_of_stage3[0]])) short(repository[4].data_of_stage2[2]);  //�����ڴ�
					else if (no == 46) new (static_cast<void*> (&Memory[repository[4].ans_of_stage3[0]])) int(repository[4].data_of_stage2[2]);    //�����ڴ�
				}
				else if (no == 51) {
					if (repository[4].data_of_stage2[2] == 8) {
						char tmp[2000];
						cin >> tmp;
						int len = strlen(tmp);
						int address = repository[4].data_of_stage2[0];
						for (int i = 0; i <= len; ++i) {
							new (static_cast<void*> (&Memory[address++])) char(tmp[i]);    //�����ڴ���
						}
					}
					else if (repository[4].data_of_stage2[2] == 4) {
						int loca = repository[4].data_of_stage2[0];
						while (Memory[loca] != '\0') {
							cout << Memory[loca++];
						}
					}
					else if (repository[4].data_of_stage2[2] == 9) {
						if (fp % 4 != 0) {
							int k = fp / 4 + 1;
							fp = k * 4;
						}
						repository[4].ans_of_stage3[0] = fp;
						fp += repository[4].data_of_stage2[0];
					}
				}
				next_data_ready[5] = true;
			}
			else next_data_ready[5] = false;
			//load: ���ڴ��ַ������ȡ��
			//store : ������д���ڴ��У����ʱ���������Ѿ��ӼĴ����ж��˳���
			ready[4] = false;
		}
	}

	void Execution() {

		while (true) {
			//std::unique_lock <std::mutex> lck(mtx);
			//lck.unlock();
			while (!ready[3] && !finish) { ; }
			if (finish) break;
			//lck.lock();

			if (!finish && data_ready[3]) {
				//repository[2] = repository[1];
				int no = repository[3].num % 100;
				(this->*excute[no])(repository[3]);//����ʹ�ú���ָ�����ʽ

				next_data_ready[4] = true;
			}
			else next_data_ready[4] = false;

			ready[3] = false;
		}
	}

	//�����޸�decode֮��ı��룡
	void Decode_Pre() {

		while (true) {
			//std::unique_lock <std::mutex> lck(mtx);
			//lck.unlock();
			while (!ready[2] && !finish) { ; }
			if (finish) break;
			//lck.lock();

			if (!finish) {
				if (!data_hazard && !ctrl_stop && data_ready[2]) {
					//repository[1] = repository[0];
					int stage;
					if (repository[2].num % 100 != 50) stage = (this->*decode[(repository[2].num / 100) % 10])(repository[2]);         //���ں�������
					else stage = 3;

					switch (stage) {
						//0���ǿ����Ҷ�ȡʧ�ܣ� 1�����ǿ��������ֶ�ȡʧ�ܣ� 2�������Ҷ�ȡ�ɹ���3���ǿ����Ҷ�ȡ�ɹ���
					case 0: next_data_hazard = data_hazard + 1; next_data_ready[3] = false; break;
					case 2: next_ctrl_stop = ctrl_stop + 1; next_data_ready[3] = true; break;
					case 1: next_data_hazard = data_hazard + 1; next_ctrl_stop = ctrl_stop + 1; next_data_ready[3] = false; break;
					case 3: next_data_ready[3] = true; break;
					}
				}
				else {
					//�����Ƿ�ctrl_stop���������ݶ��������ϵ��ˣ�data_ready[3]������Ϊ1�ˣ�
					if (data_hazard) {
						//���ʼĴ����ɹ�
						if ((this->*decode[(repository[2].num / 100) % 10])(repository[2]) >= 2) {
							next_data_hazard = data_hazard - 1;
							next_data_ready[3] = true;
						}
						else next_data_ready[3] = false;
					}
					else next_data_ready[3] = false;
				}
			}
			else next_data_ready[3] = false;

			ready[2] = false;
		}
	}

	void Ins_Fetch() {

		while (true) {
			//std::unique_lock <std::mutex> lck(mtx);
			//lck.unlock();
			while (!ready[1] && !finish) { ; }
			//lck.lock();
			if (finish) break;

			//�����Ѿ�������������֮��ľͲ���load��hazard��Ҳ����Ҫ��
			if (data_ready[1] && !finish && !ctrl_stop && !data_hazard && PC_order < text.size()) {
				repository[1].fresh();
				repository[1].instruct = text[PC_order];
				repository[1].instruct_address = PC_order;
				repository[1].num = repository[1].instruct.No;
				/*if (!next_ctrl_stop) {
					fout << PC_order << "\t" << map_out[repository[1].instruct.No % 100] << "\t" << repository[1].instruct << "\n";
					for (int i = 0; i <= 33; ++i) {
						fout << i << "\t";
					}
					fout << "\n";
					for (int i = 0; i <= 33; ++i) {
						fout << rgstr[i] << "\t";
					}
					//fout << "\n";
					//fout << "Memory 44: " << int(Memory[44]);
					fout << "\n" << "\n";
				}
				/*if (PC_order == 360) {
				/*finish = true;
				data_ready[1] = false;
				return;
					system("pause");
				}*/
				next_PC_order = PC_order + 1;
				next_data_ready[2] = true;
			}
			else {
				if (!data_hazard && !ctrl_stop) next_data_ready[2] = false;
				else next_data_ready[2] = true;
			}
			ready[1] = false;
		}
	}

	void move_storage() {
		if (next_data_ready[5]) repository[5] = repository[4];
		if (next_data_ready[4]) repository[4] = repository[3];
		if (next_data_ready[3]) repository[3] = repository[2];
		if (next_data_ready[2] && !next_data_hazard && !next_ctrl_stop && !jump) repository[2] = repository[1];
		else if (jump) next_data_ready[2] = false;//�����ת������ζ�ŵڶ�������һ�����ݲ������ڣ����ɲ���

		jump = false;
		finish = next_finish;
		ctrl_stop = next_ctrl_stop;
		data_hazard = next_data_hazard;
		PC_order = next_PC_order;

		for (int i = 0; i <= 33; ++i) {
			use_reg[i] = next_use_reg[i];
		}

		for (int i = 1; i <= 5; ++i) {
			data_ready[i] = next_data_ready[i];
		}
		
	}

	void go() {
		for (int i = 5; i >= 1; --i) {
			ready[i] = true;
		}
	}

	bool check() {
		for (int i = 1; i <= 5; ++i) {
			if (ready[i]) return 0;
		}
		return 1;
	}
};

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
	next_use_reg[obj.reg_to_be_input[0]] = 1 + use_reg[obj.reg_to_be_input[0]];
	return 3;
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
		next_use_reg[32] = 1 + use_reg[32]; next_use_reg[33] = 1 + use_reg[33];
	}
	else {
		obj.reg_to_be_input[0] = obj.instruct.no_of_reg[0];
		next_use_reg[obj.reg_to_be_input[0]] = 1 + use_reg[obj.reg_to_be_input[0]];
	}
	return 3;
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
	next_use_reg[obj.reg_to_be_input[0]] = 1 + use_reg[obj.reg_to_be_input[0]];
	return 3;
}

//��j��ת����: һ������Ĵ��� + ����Ĵ��� / ������
int performer::decode_3(performer::product& obj) {
	//������Ĵ���
	for (int i = 1; i <= 2; ++i) {
		if (obj.instruct.no_of_reg[i] != -1) {
			if (use_reg[obj.instruct.no_of_reg[i]]) return 1;
		}
	}
	//��Ϊ�����ݶ���
	for (int i = 1; i <= 2; ++i) {
		if (obj.instruct.no_of_reg[i] != -1) obj.data_of_stage2[i - 1] = rgstr[obj.instruct.no_of_reg[i]];
		else obj.data_of_stage2[i - 1] = obj.instruct.x;
	}

	obj.data_of_stage2[2] = obj.instruct.label;
	return 2;
}

// ��j��ת����: һ������Ĵ���
int performer::decode_4(performer::product& obj) {
	if (use_reg[obj.instruct.no_of_reg[1]]) return 1;
	obj.data_of_stage2[0] = rgstr[obj.instruct.no_of_reg[1]];

	obj.data_of_stage2[2] = obj.instruct.label;
	return 2;
}

//b / j / jr / jal / jalr
int performer::decode_5(performer::product& obj) {
	if (obj.instruct.no_of_reg[1] != -1) {
		if (use_reg[obj.instruct.no_of_reg[1]]) return 1;
	}
	if (obj.instruct.no_of_reg[1] != -1) {
		obj.data_of_stage2[2] = rgstr[obj.instruct.no_of_reg[1]];
	}
	else obj.data_of_stage2[2] = obj.instruct.label;

	if (obj.num % 100 == 38 || obj.num % 100 == 39) {    //jal���� / jalr
		obj.reg_to_be_input[0] = 31;
		next_use_reg[obj.reg_to_be_input[0]] = 1 + use_reg[obj.reg_to_be_input[0]];
	}
	return 2;
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
		next_use_reg[obj.reg_to_be_input[0]] = 1 + use_reg[obj.reg_to_be_input[0]];
		return 3;
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
	next_use_reg[obj.reg_to_be_input[0]] = 1 + use_reg[obj.reg_to_be_input[0]];
	return 3;
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
	return 3;
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
				return 3;
			}
		}
		else if (choice == 5) {
			obj.reg_to_be_input[0] = reg["$v0"];
			next_use_reg[reg["$v0"]] = 1 + use_reg[reg["$v0"]];
			return 3;
		}
		else if (choice == 8) {
			if (use_reg[reg["$a0"]] || use_reg[reg["$a1"]]) return 0;
			else {
				obj.data_of_stage2[0] = rgstr[reg["$a0"]];
				obj.data_of_stage2[1] = rgstr[reg["$a1"]];
				return 3;
			}
		}
		else if (choice == 9) {
			if (use_reg[reg["$a0"]]) return 0;
			else {
				obj.data_of_stage2[0] = rgstr[reg["$a0"]];
				obj.reg_to_be_input[0] = reg["$v0"];

				next_use_reg[reg["$v0"]] = 1 + use_reg[reg["$v0"]];
				return 3;
			}
		}
		else if (choice == 10) {
			//use_reg[reg["$v0"]] = true;
			next_data_ready[1] = false;
			return 3;
		}
		else if (choice == 17) {
			next_data_ready[1] = false;
			if (use_reg[reg["$a0"]]) return 0;
			else {
				obj.data_of_stage2[0] = rgstr[reg["$a0"]];
				//use_reg[reg["$a0"]] = true;
				//use_reg[reg["$v0"]] = true;
				return 3;
			}
		}
	}
}


//������ǣ� ������/ƫ���� + Ҫд��ļĴ���
//syscall�ĵ�����ʵ��
void performer::syscall(performer::product& obj) {  //�ǵøĻ�����
	switch (obj.data_of_stage2[2])
	{
	case 1:cout << obj.data_of_stage2[0]; break;   //�ȼӻ��з����������
	case 5:cin >> obj.ans_of_stage3[0]; break;
	case 4:case 8:case 9:case 10: case 17: break;
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
	//ifstream fin("array_test1-mahaojun.s");
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
	std::thread threads[5];
	threads[0] = thread(&performer::Write_Bk, &CPU);
	threads[1] = thread(&performer::Memory_Access, &CPU);
	threads[2] = thread(&performer::Execution, &CPU);
	threads[3] = thread(&performer::Decode_Pre, &CPU);
	threads[4] = thread(&performer::Ins_Fetch, &CPU);

	while (!CPU.end()) {
		
		//std::cout << "5 threads ready to race...\n";
		//ready = true; // go!
		CPU.go();//go!

		while (!CPU.check()) { ; }
		CPU.move_storage();
		
		if (CPU.end()) {
			for (auto & th : threads)
						th.join();
		}
		/*CPU.Write_Bk();
		CPU.Memory_Access();
		CPU.Execution();
		CPU.Decode_Pre();
		CPU.Ins_Fetch();
		*/
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
	//tmp_txt[loca].address = loca;
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
