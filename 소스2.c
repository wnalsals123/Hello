#include <stdio.h>zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz

#include <string.h>

#include <ctype.h>

#include <math.h>

#include <stdlib.h>



struct reg

{

	char reg_name[3];

	char reg_num[4];

}Reg[20]; // 레지스터에 대한 이름과 번호를 저장하는 구조체



struct ins

{

	char instruct[3];

	char dest[2];

	char sour[2];

	char word_type[2];

	char ins_code[3];

	char ins_len[2];

	char mod_reg[9];

}Instr[100], modInstr[100], Instr2[100], modInstr2[100]; // 각 인스트럭션의 정보를 보관하는 구조체, obj 구조체

int MaxI, Maxll, InstrP = 0;



struct symbol_tbl

{

	char symbol[10];

	char word_type[2];

	int lc;

	char data[10];

} Symbol[30], Symbol2[30];  // 심볼 테이블

int MaxS = 0;



struct sentence

{

	char label[10];

	char _operator[10];

	char operand[3][10];

} Sen, Sen2; // 원시 코드 임시 저장 구조체

int LC;

int LC2;



FILE *ObjSave;  // objcode.txt 저장

FILE *SymbolSave; // symbol.txt 저장



				  // 파일에서 레지스터와 인스트럭션의 정보를 읽는다.

void Initialize()

{

	int i = 0, j = 1;

	FILE *regi, *inst;



	regi = fopen("reg.tbl", "r");

	inst = fopen("inst.tbl", "r");

	while (!feof(regi))

	{

		fscanf(regi, "%s%s\n", Reg[i].reg_name, Reg[i].reg_num);

		i++;

	} // 레지스터 테이블을 작성한다.



	while (!feof(inst))

	{

		fscanf(inst, "%6s%2s%2s%4s%3s%2s%9s\n", Instr[j].instruct, Instr[j].dest, Instr[j].sour,

			Instr[j].word_type, Instr[j].ins_code, Instr[j].ins_len, Instr[j].mod_reg);

		j++;

	} // 명령어 테이블을 작성한다.



	while (!feof(inst))

	{

		fscanf(inst, "%6s%2s%2s%4s%3s%2s%9s\n", Instr2[j].instruct, Instr2[j].dest, Instr2[j].sour,

			Instr2[j].word_type, Instr2[j].ins_code, Instr2[j].ins_len, Instr2[j].mod_reg);

		j++;

	} // 명령어 테이블을 작성한다.

	MaxI = j - 1;

	Maxll = j - 1;

	fclose(regi);

	fclose(inst);

}



int Analyze(char *operand)

{

	int i = 0;

	char *regist[] = { "AX", "BX", "CX" ,"DX", "AL", "BL", "CL", "DL", "AH", "BH", "CH", "DH", 0x00 };

	// 레지스터 이름을 저장



	if (isdigit(operand[0])) return 0; // immediate 어드레스 모드를 지정

	else

	{

		while (regist[i] != 0x00)

		{

			if (!strcmp(operand, regist[i]))

			{

				if (i<4) return 1; // 레지스터 방식 + 워드

				else return 2; // 레지스터 방식 + 바이트

			}

			else i++;

		}

	}

	return 3;   // 메모리 지정 방식

}



#define MAX_INS 1   // 명령어의 최대 개수를 지정

int Mov_Chk(char *sen) // 오퍼랜드의 어드레스 모드를 판정

{

	int k = MaxI;

	int i = 0, j = 0, l = 0, wp = 0;

	char op[6][10];

	char *opcode[] = { "mov", "" };

	while (sen[wp] != '\n')

	{

		while (sen[wp] == ' ' || sen[wp] == '\t' || sen[wp] == ', ')wp++;// 공백, 탭, 콤마는 통과

		while (sen[wp] != ' ' && sen[wp] != '\n' && sen[wp] != ', ')

		{

			*(op[j] + i) = sen[wp];

			i++;

			wp++;

		}

		*(op[j] + i) = '\0';

		i = 0;

		j++;

	}

	i = 0;

	while (strcmp(opcode[i], ""))  // 등록된 명령어의 끝까지 비교

	{

		if (stricmp(opcode[i], op[0]))i++; //mov 명령어와 op[0]이 다르면, 다음 명령어 조회

		else

		{

			strcpy(Sen._operator, op[0]);  // 명령어를 _operator에 저장

			for (l = 1; l<j; l++)

				strcpy(Sen.operand[l - 1], op[l]); // 명령어 뒷 내용을 operand에 저장

			break;

		}

	}

	if (i == MAX_INS)//i=1이면, 명령어를 찾지 못한 것(명령어 1개 기준)

	{

		strcpy(Sen.label, op[0]);   // 이 경우 첫 단어를 레이블에 등록

		strcpy(Sen._operator, op[1]);  // 두번째 단어를 명령어에 등록

		for (l = 2; l<j; l++)

			strcpy(Sen.operand[l - 2], op[l]); // 그 다음 내용을 operand에 저장

	}// 한 문장을 분석하여 레이블, 오퍼레이터와 오퍼랜드로 분류한다.

	strcpy(Instr[0].instruct, op[0]);  // 명령어를 instruct에 저장

	switch (Analyze(op[1])) // 도착지 분석

	{

	case 0: strcpy(Instr[0].dest, "i");  // 값 즉시 입력 방식

		break;

	case 1: strcpy(Instr[0].dest, "r"); // 레지스터 방식 + 워드

		strcpy(Instr[0].word_type, "w");

		break;

	case 2: strcpy(Instr[0].dest, "r"); // 레지스터방식 + 바이트

		strcpy(Instr[0].word_type, "b");

		break;

	case 3: strcpy(Instr[0].dest, "m"); // 메모리 지정 방식

		break;

	}

	switch (Analyze(op[2]))   // 출발지 분석

	{

	case 0: strcpy(Instr[0].sour, "i"); // 값 즉시 입력 방식

		break;

	case 1: strcpy(Instr[0].sour, "r"); // 레지스터 방식 + 워드

		strcpy(Instr[0].word_type, "w");

		break;

	case 2: strcpy(Instr[0].sour, "r"); // 레지스터 방식 + 바이트

		strcpy(Instr[0].word_type, "b");

		break;

	case 3: strcpy(Instr[0].sour, "m"); // 메모리 지정 방식

		break;

	}

	while (stricmp(Instr[k].instruct, Instr[0].instruct) || strcmp(Instr[k].dest, Instr[0].dest)

		|| strcmp(Instr[k].sour, Instr[0].sour) || strcmp(Instr[k].word_type, Instr[0].word_type))

	{    // instruct, 즉 현재 명령어와,inst.tbl에서 가져온 명령어를 비교

		k--;  // 같은 값을 찾을 때까지,k를 감소하며 비교

	}

	return k; // 찾은 인덱스 k를 리턴

}



int Add_Chk(char *sen) // 오퍼랜드의 어드레스 모드를 판정

{

	int k = Maxll;

	int i = 0, j = 0, l = 0, wp = 0;

	char op[6][10];

	char *opcode[] = { "add", "" };

	while (sen[wp] != '\n')

	{

		while (sen[wp] == ' ' || sen[wp] == '\t' || sen[wp] == ', ')wp++;// 공백, 탭, 콤마는 통과

		while (sen[wp] != ' ' && sen[wp] != '\n' && sen[wp] != ', ')

		{

			*(op[j] + i) = sen[wp];

			i++;

			wp++;

		}

		*(op[j] + i) = '\0';

		i = 0;

		j++;

	}

	i = 0;

	while (strcmp(opcode[i], ""))  // 등록된 명령어의 끝까지 비교

	{

		if (stricmp(opcode[i], op[0]))i++; //add 명령어와 op[0]이 다르면, 다음 명령어 조회

		else

		{

			strcpy(Sen2._operator, op[0]);  // 명령어를 _operator에 저장

			for (l = 1; l<j; l++)

				strcpy(Sen2.operand[l - 1], op[l]); // 명령어 뒷 내용을 operand에 저장

			break;

		}

	}

	if (i == MAX_INS)//i=1이면, 명령어를 찾지 못한 것(명령어 1개 기준)

	{

		strcpy(Sen2.label, op[0]);   // 이 경우 첫 단어를 레이블에 등록

		strcpy(Sen2._operator, op[1]);  // 두번째 단어를 명령어에 등록

		for (l = 2; l<j; l++)

			strcpy(Sen2.operand[l - 2], op[l]); // 그 다음 내용을 operand에 저장

	}// 한 문장을 분석하여 레이블, 오퍼레이터와 오퍼랜드로 분류한다.

	strcpy(Instr2[0].instruct, op[0]);  // 명령어를 instruct에 저장

	switch (Analyze(op[1])) // 도착지 분석

	{

	case 0: strcpy(Instr2[0].dest, "i");  // 값 즉시 입력 방식

		break;

	case 1: strcpy(Instr2[0].dest, "r"); // 레지스터 방식 + 워드

		strcpy(Instr2[0].word_type, "w");

		break;

	case 2: strcpy(Instr2[0].dest, "r"); // 레지스터방식 + 바이트

		strcpy(Instr2[0].word_type, "b");

		break;

	case 3: strcpy(Instr2[0].dest, "m"); // 메모리 지정 방식

		break;

	}

	switch (Analyze(op[2]))   // 출발지 분석

	{

	case 0: strcpy(Instr2[0].sour, "i"); // 값 즉시 입력 방식

		break;

	case 1: strcpy(Instr2[0].sour, "r"); // 레지스터 방식 + 워드

		strcpy(Instr2[0].word_type, "w");

		break;

	case 2: strcpy(Instr2[0].sour, "r"); // 레지스터 방식 + 바이트

		strcpy(Instr2[0].word_type, "b");

		break;

	case 3: strcpy(Instr2[0].sour, "m"); // 메모리 지정 방식

		break;

	}

	while (stricmp(Instr2[k].instruct, Instr2[0].instruct) || strcmp(Instr2[k].dest, Instr2[0].dest)

		|| strcmp(Instr2[k].sour, Instr2[0].sour) || strcmp(Instr2[k].word_type, Instr2[0].word_type))

	{    // instruct, 즉 현재 명령어와,inst.tbl에서 가져온 명령어를 비교

		k--;  // 같은 값을 찾을 때까지,k를 감소하며 비교

	}

	return k; // 찾은 인덱스 k를 리턴

}



void PassI(char *buf)

{

	int i, ii;

	static int j = 0;

	static int jj = 0;



	i = Mov_Chk(buf);

	// 원시 코드의 명령어와 주소 방식을 리턴 받음.

	if (i) // mov 명령어에 대한 처리

	{

		printf("%04X:%s", LC, buf); // 주소값, 원시 코드 출력

		LC += atoi(Instr[i].ins_len); // LC=LC+ 명령어 길이

	}

	else // 기호에 대한 처리

	{

		if (!stricmp(Sen._operator, "dw"))

			strcpy(Symbol[j].word_type, "w");  // dw 라면 w를 저장

		else if (!stricmp(Sen._operator, "db"))

			strcpy(Symbol[j].word_type, "b"); // db라면 b를 저장

		strcpy(Symbol[j].symbol, Sen.label); // 레이블을 심볼에 저장

		strcpy(Symbol[j].data, Sen.operand[0]); // 레이블의 값을 저장

		Symbol[j].lc = LC;    // 주소값을 저장

		printf("%04X:%s", LC, buf);  // 주소값, 원시 코드 출력

		if (*Symbol[j].word_type == 'w') LC += 2; // 주소값을 w=2,b=1만큼 증가

		else if (*Symbol[j].word_type == 'b') LC += 1;

		j++;  //j를 증가시켜 다음번 심볼 인덱스를 맞춰줌.

		MaxS++;

	}



	ii = Add_Chk(buf);

	// 원시 코드의 명령어와 주소 방식을 리턴 받음.

	if (ii) // add 명령어에 대한 처리

	{

		// printf("%04X:%s", LC, buf); // 주소값, 원시 코드 출력

		LC2 += atoi(Instr2[ii].ins_len); // LC=LC+ 명령어 길이

	}

	else // 기호에 대한 처리

	{

		if (!stricmp(Sen2._operator, "dw"))

			strcpy(Symbol2[jj].word_type, "w");  // dw 라면 w를 저장

		else if (!stricmp(Sen2._operator, "db"))

			strcpy(Symbol2[jj].word_type, "b"); // db라면 b를 저장

		strcpy(Symbol2[jj].symbol, Sen2.label); // 레이블을 심볼에 저장

		strcpy(Symbol2[jj].data, Sen2.operand[0]); // 레이블의 값을 저장

		Symbol2[jj].lc = LC2;    // 주소값을 저장

								 // printf("%04X:%s", LC, buf);  // 주소값, 원시 코드 출력

		if (*Symbol2[jj].word_type == 'w') LC2 += 2; // 주소값을 w=2,b=1만큼 증가

		else if (*Symbol2[jj].word_type == 'b') LC2 += 1;

		jj++;  //j를 증가시켜 다음번 심볼 인덱스를 맞춰줌.

		MaxS++;

	}

}



int btoi(char *dig)

{

	int i = 0, ret = 0;



	while (*(dig + i) != '\0')

	{

		if (*(dig + i) == '1') ret += pow(2, strlen(dig + i) - 1);

		//dig+i의 길이는 dig(8)+i(0)으로 -1하면 7이다.

		// 기준점인 i가 증가하므로,7,6,5,4,3,2,1,0으로 줄어든다.

		i++;

	}

	return ret;

}



void PassII(char *buf)

{

	int i, j = 0, k = 0;

	ObjSave = fopen("objCode.txt", "a+");

	i = Mov_Chk(buf);

	if (i) //i!=0일 때, 즉 명령어일 경우,

	{

		modInstr[InstrP] = Instr[i];

		printf("%04x:%3s", LC, Instr[i].ins_code); // LC와 명령어 코드 출력

		fprintf(ObjSave, "%04x:%3s", LC, Instr[i].ins_code);

		if (!strcmp(Instr[i].dest, "r"))  // 도착지가 r이면

		{

			while (stricmp(Reg[j].reg_name, Sen.operand[0]))  // 도착지와 레지스터 이름이 같게 맞춤

				j++;

			strncpy(strchr(modInstr[InstrP].mod_reg, '?'), Reg[j].reg_num, 3);

		}



		j = 0;

		if (!strcmp(Instr[i].sour, "r"))// 출발지가 r이면

		{

			while (stricmp(Reg[j].reg_name, Sen.operand[1]))  // 출발지와 레지스터 이름이 같게 맞춤

				j++;

			strncpy(strchr(modInstr[InstrP].mod_reg, '?'), Reg[j].reg_num, 3);

			//???를 레지스터 표의 값으로 채움

		}



		if (strcmp(Instr[i].dest, "m") && strcmp(Instr[i].sour, "m")) // 도착지, 출발지가 둘다 m이 아니면

		{

			printf("%02X\t\t%s", btoi(modInstr[InstrP].mod_reg), buf);

			//Instr의 명령어 코드를 16진수 변환 후 출력, 원시 코드 출력

			fprintf(ObjSave, "%02X\t\t%s", btoi(modInstr[InstrP].mod_reg), buf);

		}

		else// 도착지 or 출발지에 기호가 들어간 경우

		{

			if (!strcmp(Instr[i].dest, "m"))  // 도착지가 m인 경우

				while (strcmp(Symbol[k].symbol, Sen.operand[0]))k++;

			// 심볼과 같은 operand를 찾음. 즉 기호를 찾음

			else if (!strcmp(Instr[i].sour, "m"))  // 출발지가 m인 경우

				while (strcmp(Symbol[k].symbol, Sen.operand[1]))k++;

			// 심볼과 같은 operand를 찾음. 즉 기호를 찾음

			if (Symbol[k].lc == (atoi(Symbol[k].data))) // 같으면, 재배치 불필요

			{

				printf(" %02X\t%04X\t%s", btoi(modInstr[InstrP].mod_reg), Symbol[k].lc, buf);

				fprintf(ObjSave, " %02X\t%04X\t%s", btoi(modInstr[InstrP].mod_reg), Symbol[k].lc, buf);

			}

			else // 다르면 재배치 필요

			{

				printf(" %02X\t%04XR\t%s", btoi(modInstr[InstrP].mod_reg), Symbol[k].lc, buf);

				fprintf(ObjSave, " %02X\t%04XR\t%s", btoi(modInstr[InstrP].mod_reg), Symbol[k].lc, buf);

				//Instr의 명령어 코드를 16진수 변환 후 출력, 원시 코드 출력

			}

		}

		LC += atoi(Instr[i].ins_len);  //LC=LC+ 명령어 길이

	}



	int ii, jj = 0, kk = 0;

	ii = Add_Chk(buf);

	if (ii) //ii!=0일 때, 즉 명령어일 경우,

	{

		modInstr2[InstrP] = Instr2[ii];

		printf("%04x:%3s", LC2, Instr2[ii].ins_code); // LC와 명령어 코드 출력

		fprintf(ObjSave, "%04x:%3s", LC2, Instr2[ii].ins_code);

		if (!strcmp(Instr2[ii].dest, "r"))  // 도착지가 r이면

		{

			while (stricmp(Reg[jj].reg_name, Sen2.operand[0]))  // 도착지와 레지스터 이름이 같게 맞춤

				jj++;

			strncpy(strchr(modInstr2[InstrP].mod_reg, '?'), Reg[jj].reg_num, 3);

		}



		jj = 0;

		if (!strcmp(Instr2[ii].sour, "r"))// 출발지가 r이면

		{

			while (stricmp(Reg[jj].reg_name, Sen2.operand[1]))  // 출발지와 레지스터 이름이 같게 맞춤

				jj++;

			strncpy(strchr(modInstr2[InstrP].mod_reg, '?'), Reg[jj].reg_num, 3);

			//???를 레지스터 표의 값으로 채움

		}



		if (strcmp(Instr2[ii].dest, "m") && strcmp(Instr2[ii].sour, "m")) // 도착지, 출발지가 둘다 m이 아니면

		{

			printf("%02X\t\t%s", btoi(modInstr2[InstrP].mod_reg), buf);

			//Instr의 명령어 코드를 16진수 변환 후 출력, 원시 코드 출력

			fprintf(ObjSave, "%02X\t\t%s", btoi(modInstr2[InstrP].mod_reg), buf);

		}

		else// 도착지 or 출발지에 기호가 들어간 경우

		{

			if (!strcmp(Instr2[ii].dest, "m"))  // 도착지가 m인 경우

				while (strcmp(Symbol2[kk].symbol, Sen2.operand[0]))kk++;

			// 심볼과 같은 operand를 찾음. 즉 기호를 찾음

			else if (!strcmp(Instr2[ii].sour, "m"))  // 출발지가 m인 경우

				while (strcmp(Symbol2[kk].symbol, Sen2.operand[1]))kk++;

			// 심볼과 같은 operand를 찾음. 즉 기호를 찾음

			if (Symbol2[kk].lc == (atoi(Symbol2[kk].data))) // 같으면, 재배치 불필요

			{

				printf(" %02X\t%04X\t%s", btoi(modInstr2[InstrP].mod_reg), Symbol2[kk].lc, buf);

				fprintf(ObjSave, " %02X\t%04X\t%s", btoi(modInstr2[InstrP].mod_reg), Symbol2[kk].lc, buf);

			}

			else // 다르면 재배치 필요

			{

				printf(" %02X\t%04XR\t%s", btoi(modInstr2[InstrP].mod_reg), Symbol2[kk].lc, buf);

				fprintf(ObjSave, " %02X\t%04XR\t%s", btoi(modInstr2[InstrP].mod_reg), Symbol2[kk].lc, buf);

				//Instr의 명령어 코드를 16진수 변환 후 출력, 원시 코드 출력

			}

		}

		LC2 += atoi(Instr2[ii].ins_len);  //LC=LC+ 명령어 길이

	}



	if (i == 0)// 기호일 경우,

	{

		k = 0;

		while (strcmp(Symbol[k].symbol, Sen.label))k++; // symbol[k]과 label가 같을 때까지 루프. 같게 만듦

		if (!strcmp(Symbol[k].word_type, "w")) // 타입이 w면

		{

			printf("%04X:%04X\t\t%s", LC, atoi(Symbol[k].data), buf);

			fprintf(ObjSave, "%04X:%04X\t\t%s", LC, atoi(Symbol[k].data), buf);

		}

		if (!strcmp(Symbol[k].word_type, "b")) // 타입이 b면

		{

			printf("%04X:%02X\t\t\t%s", LC, atoi(Symbol[k].data), buf);

			fprintf(ObjSave, "%04X:%02X\t\t\t%s", LC, atoi(Symbol[k].data), buf);

		}

		if (*Symbol[k].word_type == 'w')LC += 2;//w면 주소 값 2증가,b면 주소 값 1증가

		else if (*Symbol[k].word_type == 'b')LC += 1;

	}



	//###################################################################################

	InstrP++;

	fclose(ObjSave);

}



// 기호표 출력

void Symbol_Print()

{

	int i = 0;

	SymbolSave = fopen("symbol.txt", "w+");

	printf("\n*Symbol Table *\n");

	printf("SYMBOL\tDATA(ADDRESS)\tRELOCATION\n");

	for (i = 0; i<MaxS; i++)

	{ // symbol[k]과 label가 같을 때까지 루프. 같게 만듦

		if (!strcmp(Symbol[i].word_type, "w")) // 타입이 w

		{

			printf("%s\t%X\t\t%d\n", Symbol[i].symbol, Symbol[i].lc, (Symbol[i].lc != atoi(Symbol[i].data) ? 1 : 0));

			// 주소 값, 레이블 값, 원시 코드

			fprintf(SymbolSave, "%s\t%X\t\t%d\n", Symbol[i].symbol, Symbol[i].lc, (Symbol[i].lc != atoi(Symbol[i].data) ? 1 : 0));

		}

		if (!strcmp(Symbol[i].word_type, "b")) // 타입이 b

		{

			printf("%s\t%X\t\t%d\n", Symbol[i].symbol, Symbol[i].lc, (Symbol[i].lc != atoi(Symbol[i].data) ? 1 : 0));

			// 주소값, 레이블 값, 원시 코드

			fprintf(SymbolSave, "%s\t%X\t\t%d\n", Symbol[i].symbol, Symbol[i].lc, (Symbol[i].lc != atoi(Symbol[i].data) ? 1 : 0));

		}

	}



}





void main()

{

	char buf[50];

	FILE *in;



	in = fopen("test1.asm", "r");

	Initialize();  // 레지스터 표, 명령어 표 읽음

	printf("\nPass1:\n");

	while (1)   // 패스 1

	{

		fgets(buf, 30, in);

		if (feof(in)) break;

		PassI(buf);

	}

	Symbol_Print();  // 기호표 출력



	rewind(in);    // 포인터 초기화

	LC = 0;

	LC2 = 0;

	ObjSave = fopen("objCode.txt", "w+");

	printf("\nPass2:\n");

	while (1)   // 패스 2

	{

		fgets(buf, 30, in);

		if (feof(in)) break;

		PassII(buf);

	}

	fclose(in);

}
