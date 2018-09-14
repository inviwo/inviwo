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
import gdcm.Tag;
import java.util.Set;
import java.util.HashSet;

public class TestTag
{
  public static void main(String[] args) throws Exception
    {
      Tag t1 = new Tag(0x8,0x8);
      //System.out.println("t1: " + t1.hashCode());
      Tag t2 = new Tag(0x8,0x8);
      //System.out.println("t2: " + t2.hashCode());
      Tag t3 = new Tag(0x8,0x9);
      if( t1 == t2 )
        throw new Exception("Instances are identical" );
      if( !t1.equals(t2) )
        throw new Exception("Instances are different" );
      if( !t2.equals(t1) )
        throw new Exception("Instances are different" );
      if( t1.equals(t3) )
        throw new Exception("Instances are equals" );
      if( t1.hashCode() != t2.hashCode() )
        throw new Exception("hashCodes are different" );
      Set<Tag> s = new HashSet<Tag>();
      s.add(t1);
      s.add(t2);
      s.add(t3);
      if( s.size() != 2 )
        throw new Exception("Invalid size: " + s.size() );
      //System.out.println("compareTo: " + t1.compareTo(t2));
      //System.out.println("compareTo: " + t2.compareTo(t1));
      //System.out.println("compareTo: " + t1.compareTo(t3));
      //System.out.println("compareTo: " + t3.compareTo(t1));
      if( t1.compareTo(t2) != 0 )
        throw new Exception("Invalid compareTo: " + t1 + " vs " + t2 );
      if( t2.compareTo(t1) != 0 )
        throw new Exception("Invalid compareTo: " + t1 + " vs " + t2 );
      if( t1.compareTo(t3) >= 0 )
        throw new Exception("Invalid compareTo: " + t1 + " vs " + t3 );
      if( t3.compareTo(t1) <= 0 )
        throw new Exception("Invalid compareTo: " + t1 + " vs " + t3 );
    }
}
