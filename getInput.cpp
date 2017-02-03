/*

Gets input voltage from voltage and translates it into a MIDI isgnal

 first two intacters defines the bitrate

after that is established, a int may be sent every second to define any change in latency or position
	
	1st int defines the space between it and the previous int

 	2nd intacter defines the surface

	3rd defines the voice, fourth the effect

	5th int and a bool define the velocity

	6th, 7th and 8th the x and y and z positions of the instrument

	9th is a bit that defines the right or left hand

	10 ints and one bit == 9 bytes * 8 bits + 1 bit = 73 bits per signal

   at a data stream rate of 2kb/s, ~27 signals may be transmitted, so that's nearly 32nd notes at 60bpm (andante)

	at ~5kb/s, the throughput is ~68 signals per second, or 16th notes at 240 bpm. 

	at ~10kb/s, there are ~100 signals per second, or 32nd notes at 180, around the max throughput necessary;

	therefore, even with a very liberal buffer of 5kb/s, the necessary datastream is 15kb/s, around 1/10th the max throughput of the 802.4.15 protocol   
  
	running ~30 instruments would require approximately 52 channels, plus pit & accessories, so running in parallel would be practical



	
*/


// when using multithreaded interfaces, the atomic<T> data type does not incur race conditions and is thread safe
// bool std::atomic.is_lock_free()
// atomic_flag -- gauranteed to be lock free 


#include <iostream>
#include <string>
#include <map>
#include <list>
#include <vector>
#include <set>
#include <tuple>
#include <algorithm>
#include <inttypes.h>
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/sequence.h>
#include <thrust/copy.h>
#include <thrust/fill.h>
#include <thrust/functional.h>
#include <thrust/transform.h>
#include <thrust/replace.h>
#include <bitset>
#include <mpi.h>

using std::list;

using std::string;

using std::vector;

using std::map;

using std::pair;

using std::set;

using std::bitset;


#define read std::cin
#define say std::cout <<
#define fin << std::endl;

#define add MPI_Get_address

#define thv<int> thrust::host_vector<int> 
#define thv<bool> thrust::host_vector<bool> 
#define thv<double> thrust::host_vector<double> 
#define thv<string> thrust::host_vector<string> 
#define thv<long> thrust::host_vector<long>
#define thv<float> thrust::host_vector<float>  
#define thv<uint32_t> thrust::host_vector<uint32_t>
#define thv<uint64_t> thrust::host_vector<uint64_t>
#define thv<bitset<int> > thrust::host_vector<bitset<int> >  
#define thv<Data> thrust::host_vector<Data>

#define thp<int> thrust::host_ptr<int> 
#define thp<bool> thrust::host_ptr<bool> 
#define thp<double> thrust::host_ptr<double> 
#define thp<string> thrust::host_ptr<string> 
#define thp<float> thrust::host_ptr<float> 
#define thp<long> thrust::host_ptr<long> 
#define thp<uint32_t> thrust::host_ptr<uint32_t>
#define thp<uint64_t> thrust::host_ptr<uint64_t>
#define thp<bitset<int> > thrust::host_ptr<bitset<int> >  
#define thp<Data> thrust::host_ptr<Data>

#define tdv<int> thrust::device_vector<int> 
#define tdv<bool> thrust::device_vector<bool> 
#define tdv<double> thrust::device_vector<double> 
#define tdv<string> thrust::device_vector<string> 
#define tdv<float> thrust::device_vector<float> 
#define tdv<long> thrust::device_vector<long> 
#define tdv<uint32_t> thrust::device_vector<uint32_t> 
#define tdv<uint64_t> thrust::device_vector<uint64_t> 
#define tdv<bitset<int> > thrust::device_vector<bitset<int> >  
#define tdv<Data> thrust::device_vector<Data>

#define tdp<int> thrust::device_ptr<int> 
#define tdp<bool> thrust::device_ptr<bool> 
#define tdp<double> thrust::device_ptr<double> 
#define tdp<string> thrust::device_ptr<string>
#define tdp<float> thrust::device_ptr<float> 
#define tdp<long> thrust::device_ptr<long>  
#define tdp<uint32_t> thrust::device_ptr<uint64_t> 
#define tdp<uint64_t> thrust::device_ptr<uint32_t>
#define tdp<bitset<int> > thrust::device_ptr<bitset<int> >  
#define tdp<Data> thrust::device_ptr<Data>

#define rpi int * raw_ptrint
#define rps short * raw_ptrshort
#define rpd double * raw_ptrdouble
#define rpf float * raw_ptrfloat
#define rpl long * raw_ptrlong
#define rpb bool * raw_ptrbool
#define rp32 uint32_t * raw_ptr32
#define rp64 uint64_t * raw_ptr64
#define rpbs bitset<int> * raw_ptrbs

#define szi sizeof(int)
#define szu32 sizeof(uint32_t)
#define szu64 sizeof(uint64_t)
#define szl sizeof(long)
#define szd sizeof(double)
#define szb sizeof(bool)
#define szst sizeof(string)
#define szs sizeof(short)

#define pb push_back

#define mbyte MPI_BYTE
#define comm MPI_Comm
#define mstatus MPI_status
#define mshort MPI_UNSIGNED_SHORT
#define mint MPI_INT
#define mchart MPI_WCHAR_T
#define mchar MPI_CHAR
#define lb MPI_LB
#define ub MPI_UB


#define szo sizeof


using thrust::fill;
using thrust::sequence;
using thrust::copy;
using thrust::replace;


typedef struct struct Pad32
typedef struct struct Pad64 



//////////////////////////							///////////////////



struct Pad64(mchart * c){

bitset<16> t(c[0]);

bitset<2> fx(t.begin(), t.begin() + 1); // 4 possible effects settings
bitset<3> vox(t.begin() + 2, t.begin() + 4); // 8 possible voices
bitset<2> art(t.begin() + 5, t.begin() + 6); // 4 possible articulations
bitset<1> posit(t.begin() + 7); // 2 different surfaces (rim and drum)
bitset<3> dx = (t.begin() + 8, t.begin() + 10); // 6 units of measure from last x position
bitset<3> dy  = (t.begin() + 11, t.begin() + 13); // 6 units of measure for from last y position
bitset<2> dz  = (t.begin() + 14, t.begin() + 16); // 6 units of measure from last z position 

mshort vel = (short)c[1]; // +65k values

bitset<16> u(c[2]);

bitset<1> volt(u.begin(), u.begin() + 1);//does voltage output meet specified requirement
bitset<1> ping(u.begin() + 1, u.begin() + 2);// whether previous message was received
bitset<2> off(u.begin() + 2,  u.begin() + 4);//full automatic shutdown or on/off  
bitset<4> lumen(u.begin() + 4, u.begin() + 8);//intensity of light source
bitset<8> rgb(u.begin() +  8, u.begin() + 15);//r/g/b values
 
bitset<16> m(c[3]);

bitset<8> when(m.begin(), m.begin() + 7); // timestamp for latency calculation(s)
bitset<8> where(m.begin() + 7, n.begin() + 15); // location on pad

//	int size8 = sizeof(bitset<8>);
	
	int size8 = sizeof(bitset<8>);
	int size4 = sizeof(bitset<4>);
	int size3 = sizeof(bitset<3>);
	int size2 = sizeof(bitset<2>);
	int size1 = sizeof(bitset<1>);

	int sizeBit = sizeof(bool);
	int szShort = sizeof(mshort);

	//MPI struct variables

	int[15] displacements(sizeBit, sizeBit, sizeBit, sizeBit, sizeBit, sizeBit, sizeBit, szShort, sizeBit, sizeBit, sizeBit, sizeBit, sizeBit, sizeBit, sizeBit);

	int[15] arrayLengths(8, 2, 3, 2, 1, 3, 3, 2, 1, 1, 1, 2, 4);

	int[15] arrayOfTypes(mbit, mbit, mbit, mbit, mbit, mbit, mbit, mByte, mbit, mbit, mbit);

}; // Pad64


///////////////////					///////////////////////




struct Pad32(mchar * c){


	bitset<8> t(c[0]);

	bitset<2> fx(t.begin(), t.begin() + 1); // 4 possible effects settings
	bitset<3> vox(t.begin() + 1, t.begin() + 3); // 8 possible voices
	bitset<2> art(t.begin() + 3, t.begin() + 5); // 4 possible articulations
	bitset<1> posit(t.begin() + 6); // 2 different surfaces (rim and drum)

	bitset<8> u(c[1]);

	bitset<3> dx = (u.begin(), u.begin() + 3); // 6 units of measure from last x position
	bitset<3> dy  = (u.begin() + 3, u.begin() + 6); // 6 units of measure for from last y position
	bitset<2> dz  = (u.begin() + 6, u.begin() + 8); // 4 units of measure from last z position 

	mbyte vel(c[2]); // 128 values

	bitset<8> u(c[3]);

	bitset<1> volt(u.begin(), u.begin() + 1); // does voltage output meet specified requirement
	bitset<1> ping(u.begin() + 1, u.begin() + 2); // whether previous message was received
	bitset<2> off(u.begin() + 2,  u.begin() + 4); //full automatic shutdown or on/off  

	bitset<4> when(u.begin() + 4, u.begin() + 7); // timestamp for latency calculation(s)


	//	int size8 = sizeof(bitset<8>);
	
	int sizeBit = sizeof(bool);

	int sizeByte = sizeof(mbyte);

	// MPI struct variables
	
	int[15] displacements(sizeBit, sizeBit, sizeBit, sizeBit, sizeBit, sizeBit, sizeBit, sizeByte, sizeBit, sizeBit, sizeBit);

	int[15] arrayLengths(8, 2, 3, 2, 1, 3, 3, 2, 1, 1, 1, 2, 4);

	int[15] arrayOfTypes(mbit, mbit, mbit, mbit, mbit, mbit, mbit, mByte, mbit, mbit, mbit);


}; // Pad32







/////////////////////					///////////////////////////



class Channel{

public:


Channel(int, int){};
virtual ~Channel(int){};

int x; // bandwidth
int n; // nCores
protected:

MPI_Dataype pad32;
MPI_Datatype pad64;

// world rankings for send and receive 

// params = arrayOfBlockLengths, array_ofDisplacements, array_ofTypes, addressOfStruct
MPI_Type_create_struct();

Pad64 * initPad64(mchart * m);
Pad32 * initPad32(mchart * m);

//receive rankings

int r0 = 0;
int r1 = 1;
int r2 = 2;
int r3 = 3;

//send rankings

int s0 = 4;
int s1 = 5;
int s2 = 6;
int s3 = 7;

#include "mpi.h"

#include <stdio.h>

int main(argc,argv) 
int argc;
char *argv[];  {
int numtasks, rank, dest, source, rc, count, tag=1;  
char inmsg, outmsg='x';
MPI_Status Stat;

MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);

if (rank == 0) {
  dest = 1;
  source = 1;
  rc = MPI_Send(&outmsg, 1, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
  rc = MPI_Recv(&inmsg, 1, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat);
  } 

else if (rank == 1) {
  dest = 0;
  source = 0;
  rc = MPI_Recv(&inmsg, 1, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat);
  rc = MPI_Send(&outmsg, 1, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
  }

rc = MPI_Get_count(&Stat, MPI_CHAR, &count);
printf("Task %d: Received %d char(s) from task %d with tag %d \n",
       rank, count, Stat.MPI_SOURCE, Stat.MPI_TAG);



MPI_Finalize();
}

int Channel::send32(int sz, Pad32 p, int rank, int tag, comm c);
int Channel::send64(int sz, Pad64 p, int rank, int tag, comm c);

void Channel::setData(Data  &d, tdv<bitset<int> > &b);


}; // end class definition

Channel::Channel(int _b, int _n){

x = _b;
n = _n;

} // channel 


virtual Channel::~Channel(){};


/*
void Channel::setBit(Data  &d, bitset[64] &b){
 
rhythm0 << b[0]; // packet 0 - 7
rhythm1 << b[1]; // 
rhythm2 << b[2];
rhythm3 << b[3];
rhythm4 << b[4];
rhythm5 << b[5);
rhythm6 << b[6);
rhythm7 << b[7);


vel0 << b[8]; // packet 8 - 23 
vel1 << b[9]; // 4
vel2 << b[10]; // 8
vel3 << b[11]; // 16
vel4 << b[12]; // 32
vel5 << b[13]; // 64
vel6 << b[14]; // 128
vel7 << b[15]; // 256
vel8 << b[16]; // 512
vel9 << b[17]; // 1024
vel10 << b[18]; // 2048
vel11 << b[19]; // 4096
vel12 << b[20]; // 8196
vel13 << b[21]; // 16392
vel14 << b[22); // 32784
vel15 << b[23]; // 65568


vox0 << b[24]; // packet 24 - 27
vox1 << b[25]; //voice selection to be placed on right side of rim 
vox2 << b[26];
vox3 << b[27];


fx1 << b[28]; // packet 28 - 31
fx2 << b[29]; // fx selection to be placed on left side of rim
fx3 << b[30];
fx4 << b[31];

pad1 << b[32]; // packet 32 - 37 
pad2 << b[33];
pad3 << b[34];
pad4 << b[35];
pad5 << b[36];
pad6 << b[37];


l1 << b[38]; // packet 38 - 40
l2 << b[39]; // voltages
l3 << b[40];

r1 << b[41]; // packet 41 - 43
r2 << b[42];
r3 << b[43];

x1 << b[44]; // packet 44 - 47
x2 << b[45]; // position of unit
x3 << b[46];
x4 << b[47];

y1 << b[48]; // packet 48 - 51
y2 << b[49];
y3 << b[50];
y4 << b[51];

z1 << b[52]; // packet 52 - 55
z2 << b[53];
z3 << b[54];
z4 << b[55];


padRx << b[56]; // packet 56 - 57
padRy << b[57]; //position on pad

padLx << b[58]; // packet 58 - 59
padLy << b[59]; 

padRx2 << b[60]; // packet 60 - 61
padRy2 << b[61]; // cross check position on pad

padLx2 << b[62]; // packet 62 - 63
padLx2 << b[63];


return d;

}
*/

//map of string->pair<int (index) index to change, (bool) boolean to change "to", so if bool is true, change to 1

Pad64 * Channel::initPad64(mchart * m){

Pad64 * p(m);

	int fxA, voxA, artA, positA, dxA, dyA, dzA, velA, voltA, pingA, offA, lumenA, rgbA, whenA, whereA;

	int fx_offset, art_offset, vox_offset, dx_offset, dy_offset, dz_offset, where_offset, when_offset, vel_offset;
	int dy_offset, dx_offset, dz_offset, off_offset, rgb_offset, lumen_offset, ping_offset, volt_offset, posit_offset;

	int pA;

	add(&p, &pA);

	add(&p->fx, &fxA);
	add(&p->vox, &voxA);
	add(&p->art, &artA);
	
	add(&p->volt, &voltA);
	add(&p->ping, &pingA);

	add(&p->lumen, &lumenA);
	add(&p->rgb, &rgbA);

	add(&p->when, &whenA);
	add(&p->where, &whereA);

	add(&p->vel, &velA);
	add(&p->posit, &positA);

	add(&p->dx, &dxA);
	add(&p->dy, &dyA);
	add(&p->dz, &dzA);


return p;

} // init pad64



Pad32 * Channel::initPad32(mchar * m){

	Pad32 * p(m);

	int fxA, voxA, artA, positA, dxA, dyA, dzA, velA, voltA, pingA, offA, whenA;

	int fx_offset, art_offset, vox_offset, dx_offset, dy_offset, dz_offset, where_offset, when_offset, vel_offset;
	int dy_offset, dx_offset, dz_offset, off_offset, rgb_offset, lumen_offset, ping_offset, volt_offset, posit_offset;

	int pA;

	add(&p, &pA);

	add(&p.fx, &fxA);
	add(&p.vox, &voxA);
	add(&p.art, &artA);

	add(&p.volt, &voltA);
	add(&p.ping, &pingA);

	add(&p.when, &whenA);

	add(&p.posit, &positA);

	add(&p.dx, &dxA);
	add(&p.dy, &dyA);
	add(&p.dz, &dzA);

	add(&p.vel, &velA);

	
return p;

} // initPad32
 


void Channel::setBit32(string name, short index, Pad32 &p){
	
		p->name[index] = p->name[!index];

} // end set bit 32



void Channel::setBit64(string name, short index, Pad64 &p){

	p->name[index] = p->name[!index];

} // set bit 64


int Channel::send(1, char * temp, int band, int rank, int tag, comm c){

			if(band == 32) pad32 * p = initPad32(temp);
			else pad64 * p = initPad64(temp);

			return MPI_Receive( p, 1, rank, tag, c, MPI_Send( p, arraySize, byte, rank, tag, c) ) ; // NOTE

} // send & receive


//ascii has 128 intacters in its libraries, so a three char string represents 2,097,152 values, a two int string with a signed bit ~4.2M values
//a four string int 268,435,456 values


//an ascii intacter would represent a certain channel input -- tone, there are 128 letters of an ascii library, so having two letters and a signed bit gives 256 possibilities


//input stream of unicode characters for a 32 bit bandwidth signal


int main(int argv, char ** argc){


	Channel * x( to_string(argc[0]) );

	//1st arg in argc is bitrate / stream
	//2nd arg in argc is either 32 or 64 bit
 
	string bandWidth = argc[0];

	// params = arrayOfBlockLengths, array_ofDisplacements, array_ofTypes, addressOfStruct
	
	int nCores = n_processors();

	char[x->x] packet;

	MPI_Init(&argc, &argv);	

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	comm om;

		while(read, argc[index]){ 

			for(int o = 0; o < x->c; o++){

				packet[o] = argc[index + o];

				if( band == 32 ){
			
					if(x->send(x->x, 1, o, int tag, om) == MPI_Receive_Success) continue;
					else say "error in MPI send, index == " << index fin

													} // if 32

				if( band == 64 ){
		
				
					if(x->send(x->x, 1, o, int tag, om) == MPI_Receive_Success) continue;
					else say "error in MPI send, index == " << index fin
								
									} // if 64

			MPI_Type_create_struct(p->displacements, p->arrayLengths, p->arrayOfTypes, &p);

	} // while read


}// main 
