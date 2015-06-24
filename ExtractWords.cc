using namespace std;

#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include "zlib.h"
#include <iomanip>

void GetDataStream( string, std::stringstream&);
void SetDataStream( string, std::stringstream&, bool);


int main(int argc, char **argv)
{
    //Extracts user Inputs
    string inFileName, dummy;
    streamoff startChar;
    long numWords;
    stringstream stream;

    if(argc==4)
    {
        stream << argv[1] << ' ' << argv[2] << ' ' << argv[3];
        stream >> inFileName >> startChar >> numWords;
    }
    else
    {
        cout << "Incorrect number of inputs; give the file name and the start character position and number of words you want to extract" << endl;
        return 1;
    }

    stream.clear();
    stream.str("");

    GetDataStream( inFileName, stream);
    if(!stream.good())
    {
        cout << "Error: couldn't open file: " << inFileName << endl;
        return 1;
    }
    stream.seekg(startChar, stream.beg);

    for(int i=0; i<numWords; i++)
    {
        stream >> dummy;
        cout << dummy << endl;
        if(!stream.good())
        {
            cout << "Error: Only made it to word " << i << endl;
            return 1;
        }
    }

    return 0;
}

void GetDataStream( string filename, std::stringstream& ss)
{
   string* data=NULL;
   std::ifstream* in=NULL;
   //string compfilename(filename);

   if(filename.substr((filename.length()-2),2)==".z")
   {
        in = new std::ifstream ( filename.c_str() , std::ios::binary | std::ios::ate );
   }

   if ( in!=NULL && in->good() )
   {
// Use the compressed file
      uLongf file_size = (uLongf)(in->tellg());
      in->seekg( 0 , std::ios::beg );
      Bytef* compdata = new Bytef[ file_size ];

      while ( *in )
      {
         in->read( (char*)compdata , file_size );
      }

      uLongf complen = (uLongf) ( file_size*4 );
      Bytef* uncompdata = new Bytef[complen];

      while ( Z_OK != uncompress ( uncompdata , &complen , compdata , file_size ) )
      {
         delete[] uncompdata;
         complen *= 2;
         uncompdata = new Bytef[complen];
      }
      delete [] compdata;
      //                                 Now "complen" has uncomplessed size
      data = new string ( (char*)uncompdata , (long)complen );
      delete [] uncompdata;
   }

   else
   {
// Use regular text file
      std::ifstream thefData( filename.c_str() , std::ios::in | std::ios::ate );
      if ( thefData.good() )
      {
         int file_size = thefData.tellg();
         thefData.seekg( 0 , std::ios::beg );
         char* filedata = new char[ file_size ];
         while ( thefData )
         {
            thefData.read( filedata , file_size );
         }
         thefData.close();
         data = new string ( filedata , file_size );
         delete [] filedata;
      }
      else
      {
// found no data file
//                 set error bit to the stream
         ss.setstate( std::ios::badbit );
         cout << endl << "### failed to open ascii file " << filename << " ###" << endl;
      }
   }
   if (data != NULL)
   {
        ss.str(*data);
        if(data->back()!='\n')
            ss << "\n";
        ss.seekg( 0 , std::ios::beg );
    }

   if(in!=NULL)
   {
        in->close();
        delete in;
   }

    if(data!=NULL)
        delete data;
}


void SetDataStream( string filename , std::stringstream& ss, bool ascii )
{
    //bool cond=true;
   if (!ascii)
   {
        string compfilename(filename);

        if(compfilename.back()!='z')
            compfilename += ".z";

       std::ofstream* out = new std::ofstream ( compfilename.c_str() , std::ios::binary | std::ios::trunc);
       if ( ss.good() )
       {
       //
    // Create the compressed file
          ss.seekg( 0 , std::ios::end );
          uLongf file_size = (uLongf)(ss.tellg());
          ss.seekg( 0 , std::ios::beg );
          Bytef* uncompdata = new Bytef[ file_size ];

          while ( ss ) {
              ss.read( (char*)uncompdata , file_size );
          }

          uLongf complen = compressBound(file_size);

          Bytef* compdata = new Bytef[complen];

          if ( Z_OK == compress ( compdata , &complen , uncompdata , file_size ) )
          {
            out->write((char*)compdata, (long)complen);
            if (out->fail())
            {
                cout << endl << "writing the compressed data to the output file " << compfilename << " failed" << endl
                    << " may not have permission to delete an older version of the file" << endl;
            }
          }
          else
          {
            cout << endl << "compressing the data failed" << endl;
          }

          delete [] uncompdata;
          delete [] compdata;
       }
       else
       {
            cout << endl << "### failed to write to binary file ###" << endl;
       }

       out->close(); delete out;
   }
   else
   {
// Use regular text file
    string compfilename(filename);

    if(compfilename.substr((compfilename.length()-2),2)==".z")
    {
        compfilename.pop_back();
        compfilename.pop_back();
    }

      std::ofstream out( compfilename.c_str() , std::ios::out | std::ios::trunc );
      if ( ss.good() )
      {
         ss.seekg( 0 , std::ios::end );
         int file_size = ss.tellg();
         ss.seekg( 0 , std::ios::beg );
         char* filedata = new char[ file_size ];
         while ( ss ) {
            ss.read( filedata , file_size );
            if(!file_size)
            {
                cout << "\n #### Error the size of the stringstream is invalid ###" << endl;
                break;
            }
         }
         out.write(filedata, file_size);
         if (out.fail())
        {
            cout << endl << "writing the ascii data to the output file " << compfilename << " failed" << endl
                 << " may not have permission to delete an older version of the file" << endl;
        }
         out.close();
         delete [] filedata;
      }
      else
      {
// found no data file
//                 set error bit to the stream
         ss.setstate( std::ios::badbit );

         cout << endl << "### failed to write to ascii file " << compfilename << " ###" << endl;
      }
   }
   ss.str("");
}
