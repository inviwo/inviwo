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
import java.util.SortedSet;
import java.util.TreeSet;
import java.util.Set;
import java.util.HashSet;
import java.util.List;
import java.util.ArrayList;

public class TestReaderUpToTag
{
  private static void PrintTagSet( Set<Tag> s ) {
    for( Tag t : s ) {
      System.out.println("Print: " + t);
    }
  }

  private static SortedSet<Tag> getTagSet() {
    Tag tags[] = {
      new Tag(0x8,0x8),
      new Tag(0x8,0x16),
      new Tag(0x8,0x18),
      new Tag(0x8,0x60),
      new Tag(0x10,0x20),
      new Tag(0x28,0x2),
      new Tag(0x28,0x10),
      new Tag(0x28,0x11),
      new Tag(0x10,0x10) };
    SortedSet<Tag> tagSet = new TreeSet<Tag>();
    for( Tag tag : tags ) {
      tagSet.add( tag );
    }
    return tagSet;
  }

  public static void main(String[] args) throws Exception {

    long nfiles = Testing.GetNumberOfFileNames();

    SortedSet<Tag> tagSet = getTagSet();
    PrintTagSet( tagSet );
    Tag last = tagSet.last();
    System.out.println("last: " + last);
    TagSetType skip = new TagSetType();
    Tag pixelData = new Tag(0x7fe0,0x0010);
    skip.insert( pixelData );
    Trace.DebugOff();
    Trace.WarningOff();
    StringFilter sf = new StringFilter();
    List<Set<Tag>> l = new ArrayList<Set<Tag>>();
    for( long i = 0; i < nfiles; ++i ) {
      Set<Tag> s = new HashSet<Tag>();
      String filename = Testing.GetFileName( i );
      if( true || filename.contains( "ExplicitVRforPublicElementsImplicitVRforShadowElements" )
          || filename.contains( "SIEMENS_SOMATOM-12-ACR_NEMA-ZeroLengthUs" )
        ) {
        Reader reader = new Reader();
        reader.SetFileName( filename );
        if ( !reader.ReadUpToTag(pixelData, skip) )
        {
          throw new Exception("Could not read: " + filename );
        }
        System.out.println("reading: " + filename);
        File file = reader.GetFile();
        sf.SetFile( file );
        DataSet ds = file.GetDataSet();
        for( Tag tag : tagSet ) {
          if( ds.FindDataElement( tag ) ) {
            s.add( tag );
            DataElement de = ds.GetDataElement( tag );
            System.out.println("de: " + tag + " -> " + sf.ToString( de ) );
          }
        }
      }
      l.add( s );
   //   PrintTagSet( s );
    System.gc ();
    }
    System.gc ();
    System.runFinalization ();
    for( Set<Tag> s : l ) {
      PrintTagSet( s );
    }
    PrintTagSet( tagSet );
  }
}
