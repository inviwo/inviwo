/*=========================================================================

  Program: GDCM (Grassroots DICOM). A DICOM library

  Copyright (c) 2006-2011 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*
 */
import gdcm.*;

public class TestStringFilter
{
  public static void main(String[] args) throws Exception
    {
/*
    System.out.println("PATH : "
      + System.getProperty("java.library.path"));
    Properties p = System.getProperties();
    Enumeration keys = p.keys();
    while (keys.hasMoreElements()) {
      String key = (String)keys.nextElement();
      String value = (String)p.get(key);
      System.out.println(key + ": " + value);
    }
*/

    long nfiles = Testing.GetNumberOfFileNames();
    Trace.DebugOff();
    Trace.WarningOff();

    for( long i = 0; i < nfiles; ++i )
      {
      String filename = Testing.GetFileName( i );
      if( filename.contains( "MR_Philips_Intera_No_PrivateSequenceImplicitVR.dcm" )) {
        //System.out.println("Success reading: " + filename );
        Reader reader = new Reader();
        reader.SetFileName( filename );
        if ( !reader.Read() )
          {
          throw new Exception("Could not read: " + filename );
          }
	File file = reader.GetFile();
	DataSet ds = file.GetDataSet();
	Tag t1 = new Tag(0x2005,0xe229);
	StringFilter sf = new StringFilter();
	sf.SetFile( file );
	if( ds.FindDataElement( t1 ) ) {
	  DataElement de1 = ds.GetDataElement(t1);
          System.out.println("de1: " + de1 );
	  String s1 = sf.ToString( de1 );
          System.out.println("s1: " + s1 );
	  float array[] = { 5.1f, -4.9f };
	  de1.SetArray( array, array.length );
          System.out.println("de1: " + de1 );
	  s1 = sf.ToString( de1 );
          System.out.println("s1: " + s1 );
	}
	Tag t2 = new Tag(0x18,0x2043);
	float array[] = { 2.5f, -1.6f };
	DataElement de = new DataElement(t2);
	//de.SetVR( new VR(VR.VRType.FL) );
	de.SetArray( array, array.length );
	String s = sf.ToString( de );
        System.out.println("s: " + s );
	ds.Replace( de );
	if( ds.FindDataElement( t2 ) ) {
	  DataElement de2 = ds.GetDataElement(t2);
          System.out.println("de2: " + de2 );
	  String s2 = sf.ToString( de2 );
          System.out.println("s2: " + s2 );
	  //float array[] = { 51, 49 };
	  //de1.SetArray( array, array.length );
          //System.out.println("de1: " + de1 );
	  //s1 = sf.ToString( de1 );
          //System.out.println("s1: " + s1 );
	  //ds.Replace( de1 );
	}

	Writer writer = new Writer();
	writer.SetFile( reader.GetFile() );
	writer.SetFileName( "testsf.dcm" );
        if ( !writer.Write() )
          {
          throw new Exception("Could not write: " + filename );
          }
        }
      }
    }
}
