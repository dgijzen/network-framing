#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

//////////////////////////////////////////////////////////////////
const int ASCII = 48;
const int ASCII_TWO = 39;
const int DEST_ADDRESS = 4;
const int SRC_ADDRESS = 4;
const int LENGTH_OF_FRAME = 8;
const int CHECK_LENGTH = 2;

//////////////////////////////////////////////////////////////////
struct Frame {//Struct can only be for plain data. Alternative would be
  //a class and overload the cin operator
  static const int DEST = DEST_ADDRESS;
  static const int SRC = SRC_ADDRESS;
  static const int LENGTH = LENGTH_OF_FRAME;
  static const int CHECK = CHECK_LENGTH;
  static const int MAX_LOAD = 228;
  static const int MAX_FRAME = DEST+SRC+LENGTH+MAX_LOAD+CHECK;

  unsigned char frame_dest[DEST];
  unsigned char frame_src[SRC];
  unsigned char frame_length[LENGTH];
  unsigned char frame_sum[CHECK];
  unsigned char frame_payload[MAX_LOAD];

  string message; //for convience string, would be better to leave it out and stick to unsigned char[]

  int payload; //payload is the actual message in the frame
  int length; //holds the value of the length of the total message in bytes
  int checksum; //holds the value of the checksum

  bool checksum_is_ok = false;
};

void check_checksum(Frame& obj, const int int_frame[], int& temp_mod);
void get_frame_length(Frame& obj);
void get_payload_length(Frame& obj);
void get_message(Frame& obj, const int input[]);

//function to return the length of the frames in bytes
void get_frame_length(Frame &obj){
  int power = obj.LENGTH -1;  //position in place is 16^x
  int temp_value;
  for(int i=0; i<obj.LENGTH; ++i){
    temp_value = obj.frame_length[i]; //starting from the most important bit to the least
    temp_value -= ASCII;
    obj.length += pow(16, power) * temp_value; //since it is hex, use power of 16
    --power;
  }
  if(obj.length>73){//Max frame exceeds 73 bytes on second input
    //TODO: throw something mean
    cout<<"frame was bigger than"<<endl;
  }
}

//function to get the length of the payload
void get_payload_length(Frame& obj){
  obj.payload = (obj.length*2)-(obj.DEST+obj.SRC+obj.LENGTH+obj.CHECK); //the length of the payload is...
}

//function to check the checksum against the calculated remainder
void check_checksum(Frame& obj, const int int_frame[], int& temp_mod){
  int mod_remainder = 0;
  obj.checksum = (16*int_frame[obj.CHECK-2])+int_frame[obj.CHECK-1];
  if(obj.checksum != temp_mod){
    //TODO: error handling
    cout<<"not a correct checksum"<<endl;
  }
  else{
    obj.checksum_is_ok = true;
  }
}

//function to get the "real message" for output
void get_message(Frame& obj, const int input[]){
  unsigned char sum;
  for(int u=0; u<obj.payload; u+=2){
    sum = (16*input[u])+input[u+1];
    obj.message+=sum;
  }
}

/////////////////////////////////////////////////////////////////////////////
void transform_to_decimal(const unsigned char input[], const int &p, int *int_frame){
  for(int i=0; i<p;++i){
    if(ASCII+10<input[i]){ //48+10 = 9 in the ascii table
      int_frame[i]=input[i] - ASCII-ASCII_TWO; //in this case we are at a-f, so subtract 48+39 to get decimal
    }
    else{
      int_frame[i]=input[i]-ASCII; //subtract 48 to get decimal value
    }
  }
}

//function TODO ? make it overflow resilient
int get_modulo_remainder(const int int_frame[], const int &a, const int &mod){
  int mod_remainder = mod;
  //check the checksum:
  for(int u=0; u<a; u+=2){ //use two char to form one byte
    int variable = 0;
    variable = (16*int_frame[u]) + int_frame[u+1];
    variable += mod_remainder; //calculate the decimal value
    mod_remainder = variable % 128; //calculate the mod 128, and leave the remainder
  }
  return mod_remainder;
}

/////////////////////////////////////////////////////////////////////////////
int main(){
  vector<Frame> VF;
  bool input_present = false;
  char own_address[DEST_ADDRESS];

  // open a file in read mode.
  ifstream infile;
  infile.open("input-tea.input");
  cout << "Reading from the file" << endl;
  input_present = true;

  //get our own address from the input file.
  for(int i=0; i<DEST_ADDRESS; ++i){
    cin>>own_address[i];
  }

  //start reading the input, frame by frame
  while(input_present){
      Frame frame;
      frame.payload = 0;
      frame.length = 0;
      frame.message = "";
      //start buffering the input and check corresponding destination
      for(int i=0; i<DEST_ADDRESS; ++i){
        cin>>frame.frame_dest[i]; //input to frame for 4 characters
        if(frame.frame_dest[i]!=own_address[i]){ //check against own_address
          cout<<"You should have thrown something, address is wrong"<<endl;
        }
      }

      //continue buffering to get length
      for(int i=0; i<SRC_ADDRESS; ++i){
        cin>>frame.frame_src[i];
      }
      //continue buffering to get length
      for(int i=0; i<LENGTH_OF_FRAME; ++i){
        cin>>frame.frame_length[i];
      }

      //get the total length of the frame
      get_frame_length(frame);
      get_payload_length(frame);

      //continue buffering the complete message
      for(int i=0; i<frame.payload; ++i){
        cin>>frame.frame_payload[i];
      }

      //continue buffering to get the checksum
      for(int i=0; i<CHECK_LENGTH; ++i){
        cin>>frame.frame_sum[i];
      }

      //create integer arrays
      int int_dest[DEST_ADDRESS];
      transform_to_decimal(frame.frame_dest, DEST_ADDRESS, int_dest);
      int int_src[SRC_ADDRESS];
      transform_to_decimal(frame.frame_src, SRC_ADDRESS, int_src);
      int int_length[LENGTH_OF_FRAME];
      transform_to_decimal(frame.frame_length, LENGTH_OF_FRAME, int_length);
      int int_payload[frame.payload];
      transform_to_decimal(frame.frame_payload, frame.payload, int_payload);
      int int_sum[CHECK_LENGTH];
      transform_to_decimal(frame.frame_sum, CHECK_LENGTH, int_sum);

      //get the remainder of each
      int temp_mod = 0;
      temp_mod = get_modulo_remainder(int_dest, DEST_ADDRESS, temp_mod);
      temp_mod = get_modulo_remainder(int_src, SRC_ADDRESS, temp_mod);
      temp_mod = get_modulo_remainder(int_length, LENGTH_OF_FRAME, temp_mod);
      temp_mod = get_modulo_remainder(int_payload, frame.payload, temp_mod);
      //modulu excludes the checksum.
      check_checksum(frame, int_sum, temp_mod);

      //get the string message
      get_message(frame, int_payload);
      //message get stored inside a vector
      VF.push_back(frame);

      //check for end of line
      char c_temp = cin.peek();
      if(!isdigit(c_temp) && !isalpha(c_temp)){
        input_present = false;
      }
  }

  cout<<"size of"<<VF.size()<<endl;
  for(int i=0; i<VF.size(); i++){
    for(int j=0; j<DEST_ADDRESS; j++){
      cout<<VF[i].frame_dest[j];
    }
    cout<<" ";
    for(int j=0; j<SRC_ADDRESS; j++){
      cout<<VF[i].frame_src[j];
    }
    cout<<" "<<VF[i].length<<" ";
    cout<<VF[i].message<<endl;
  }

  return 0;
}
