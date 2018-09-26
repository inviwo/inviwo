# Documentation
# http://www.stack.nl/~dimitri/doxygen
# http://www.stack.nl/~dimitri/doxygen/commands.html#cmdsa
if(GDCM_DOCUMENTATION)

  find_package(Doxygen REQUIRED)

  if(GDCM_DOXYGEN_NO_FOOTER)
    set(GDCM_HTML_FOOTER)
  else()
    # Otherwise use gdcm/piwik/sf.net footer:
    set(GDCM_HTML_FOOTER ${GDCM_SOURCE_DIR}/Utilities/doxygen/footer.html)
  endif()
  configure_file(
    ${GDCM_SOURCE_DIR}/Utilities/doxygen/doxyfile.in
    ${GDCM_BINARY_DIR}/Utilities/doxygen/Doxyfile
    )

  file(GLOB_RECURSE headerfiles
    "${GDCM_SOURCE_DIR}/Source/*.h"
    "${GDCM_SOURCE_DIR}/Wrapping/*.h"
    "${GDCM_SOURCE_DIR}/Utilities/VTK/*.h"
    #"${GDCM_SOURCE_DIR}/Utilities/doxygen/man/*.dox"
  )
  list(REMOVE_ITEM headerfiles
    "${GDCM_SOURCE_DIR}/Source/DataDictionary/gdcmTagKeywords.h"
    "${GDCM_SOURCE_DIR}/Source/DataDictionary/gdcmTagToType.h"
  )
  #message(${headerfiles})
  # We are depending only on header files and README.txt but other files
  # could be needed for complete dependencies
  file(GLOB_RECURSE examplefiles
    "${GDCM_SOURCE_DIR}/Examples/*.cxx"
    "${GDCM_SOURCE_DIR}/Examples/*.cs"
    "${GDCM_SOURCE_DIR}/Examples/*.java"
    "${GDCM_SOURCE_DIR}/Examples/*.py"
    )
  file(GLOB_RECURSE vtkexamplefiles
    "${GDCM_SOURCE_DIR}/Utilities/VTK/Examples/*.cxx"
    "${GDCM_SOURCE_DIR}/Utilities/VTK/Examples/*.cs"
    "${GDCM_SOURCE_DIR}/Utilities/VTK/Examples/*.java"
    "${GDCM_SOURCE_DIR}/Utilities/VTK/Examples/*.py"
    )
  set(DOXYFILE_EXAMPLES)
  foreach(file ${examplefiles} ${vtkexamplefiles})
    get_filename_component(f ${file} NAME)
    set(DOXYFILE_EXAMPLES "${DOXYFILE_EXAMPLES}\n\\example ${f}")
  endforeach()
  configure_file(
    ${GDCM_SOURCE_DIR}/Utilities/doxygen/TestsList.txt.in
    ${GDCM_BINARY_DIR}/Utilities/doxygen/TestsList.txt
    @ONLY
  )
  if(GDCM_MINOR_VERSION MATCHES "[02468]$")
    set(GDCM_DOC_PDF_LINK
      "http://gdcm.sourceforge.net/${GDCM_MAJOR_VERSION}.${GDCM_MINOR_VERSION}/gdcm-${GDCM_VERSION}.pdf"
    )
    set(GDCM_DOC_TARBALL_LINK
      "http://gdcm.sourceforge.net/${GDCM_MAJOR_VERSION}.${GDCM_MINOR_VERSION}/gdcm-${GDCM_VERSION}-doc.tar.gz"
    )
  else()
    set(GDCM_DOC_PDF_LINK
      "http://gdcm.sourceforge.net/gdcm-${GDCM_VERSION}.pdf"
    )
    set(GDCM_DOC_TARBALL_LINK
      "http://gdcm.sourceforge.net/gdcm-${GDCM_VERSION}-doc.tar.gz"
    )
  endif()
  configure_file(
    ${GDCM_SOURCE_DIR}/Utilities/doxygen/README.txt.in
    ${GDCM_BINARY_DIR}/Utilities/doxygen/README.txt
    @ONLY
  )
  set(GDCM_DOC_TARBALL
    ${CMAKE_CURRENT_BINARY_DIR}/gdcm-${GDCM_VERSION}-doc.tar.gz
  )
  # epstopdf is needed: sudo apt-get install texlive-extra-utils
  # pdflatex is needed: sudo apt-get install texlive-latex-extra
  add_custom_command(
    OUTPUT   ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
             ${CMAKE_CURRENT_BINARY_DIR}/latex/Makefile
    # let's create the tarball within the same custom command to avoid complex dep. rules.
             ${GDCM_DOC_TARBALL}
             ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.tex # output #1 (fake)
    # 1. first thing first let's run doxygen
    COMMAND  ${DOXYGEN}
    ARGS     ${GDCM_BINARY_DIR}/Utilities/doxygen/Doxyfile
    # 2. tarball gen.
    COMMAND  ${CMAKE_COMMAND}
    ARGS     -E tar cfz ${GDCM_DOC_TARBALL} html
    DEPENDS  ${GDCM_BINARY_DIR}/Utilities/doxygen/Doxyfile
             ${GDCM_BINARY_DIR}/Utilities/doxygen/README.txt
             ${GDCM_BINARY_DIR}/Utilities/doxygen/TestsList.txt
             ${headerfiles}
    COMMENT  "GDCM: Creating doxygen doc + tarball"
    #WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

  # PDF building part:
  # When building latex, we have to run a custom command to produce the pdf file:
  if(UNIX AND GDCM_PDF_DOCUMENTATION)
    find_program(PDFOPT_EXECUTABLE pdfopt)
    find_program(SED_EXECUTABLE sed)
    find_package(LATEX REQUIRED)
    mark_as_advanced(PDFOPT_EXECUTABLE SED_EXECUTABLE)
    # Let's customize the pdf tags a little usind sed:
    # Apparently egrep is also needed...
    # BAD: there is a circular dependency where refman.tex depend on refman.tex in the sed steps...
    # hack our way in anyway by simply removing the dep...
    #add_custom_command(
    #  ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/patchtex.cmake
    #)
    # TODO, foreach(*.tex)
    # sed -i -e "/home/mmalaterre/Projects/" ...
    string(REPLACE "/" "\\/" sed_gdcm_source_dir ${GDCM_SOURCE_DIR})
    #message(${sed_gdcm_source_dir})
    add_custom_command(
      OUTPUT   #${CMAKE_CURRENT_BINARY_DIR}/latex/refman.tex # output #1 (fake)
               ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf # output #2
      # Command #1
      COMMAND  ${SED_EXECUTABLE}
      ARGS     -i.tmp -e "'s/]{hyperref}/]{hyperref}\\\\hypersetup{pdftitle={GDCM ${GDCM_VERSION} Reference Manual},pdfauthor={Mathieu Malaterre and co.},pdfsubject={Grassroots DICOM API reference},pdfkeywords={GDCM,DICOM,Network,Query\\/Retrieve,JPEG,Lossless JPEG,JPEG-LS,J2K,JPEG 2000,RLE},pdfpagemode={UseOutlines},bookmarks,bookmarksopen,pdfstartview={FitH},backref,colorlinks,linkcolor={black},citecolor={black},urlcolor={black},baseurl={http:\\/\\/gdcm.sourceforge.net}}\\\\hyperbaseurl{http:\\/\\/gdcm.sourceforge.net}/g'" ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.tex
      # Command #2
      COMMAND  ${SED_EXECUTABLE}
      ARGS     -i.tmp -e "'s/${sed_gdcm_source_dir}/gdcm/g'" ${CMAKE_CURRENT_BINARY_DIR}/latex/*.tex
      # Command #3
      COMMAND  ${CMAKE_MAKE_PROGRAM}
      DEPENDS  #${CMAKE_CURRENT_BINARY_DIR}/latex/refman.tex
${CMAKE_CURRENT_BINARY_DIR}/latex/Makefile
    #${GDCM_DOC_TARBALL}
      COMMENT  "GDCM: Creating (patched) pdf of documentation"
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/latex
    )
    if(PDFOPT_EXECUTABLE)
      add_custom_command(
        OUTPUT   ${CMAKE_CURRENT_BINARY_DIR}/latex/gdcm-${GDCM_VERSION}.pdf
        COMMAND  ${PDFOPT_EXECUTABLE}
        ARGS     ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf
                 ${CMAKE_CURRENT_BINARY_DIR}/latex/gdcm-${GDCM_VERSION}.pdf
        DEPENDS  ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf
        COMMENT  "GDCM: Creating optimized pdf version of documentation"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/latex
        )
    else()
      add_custom_command(
        OUTPUT   ${CMAKE_CURRENT_BINARY_DIR}/latex/gdcm-${GDCM_VERSION}.pdf
        COMMAND  ${CMAKE_COMMAND} -E copy
        ARGS     ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf
                 ${CMAKE_CURRENT_BINARY_DIR}/latex/gdcm-${GDCM_VERSION}.pdf
        DEPENDS  ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf
        COMMENT  "GDCM: Creating unoptimized pdf version of documentation"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/latex
        )
    endif()
    # add target to 'ALL'
    add_custom_target(GDCMDoxygenPDF
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/latex/gdcm-${GDCM_VERSION}.pdf
      COMMENT  "GDCM: Executing GDCMDoxygenPDF"
    )
    install(FILES
      ${CMAKE_CURRENT_BINARY_DIR}/latex/gdcm-${GDCM_VERSION}.pdf
      DESTINATION ${GDCM_INSTALL_DOC_DIR}
    )
    add_dependencies(DoxygenDoc GDCMDoxygenPDF)
  else()
    # make DoxygenDoc depends on the final tarball thus all file are garantee to be generated
    add_custom_target(GDCMDoxygenDoc
      DEPENDS ${GDCM_DOC_TARBALL}
      COMMENT  "GDCM: Executing GDCMDoxygenDoc"
    )
    add_dependencies(DoxygenDoc GDCMDoxygenDoc)
  endif()

  # Install html pages:
  #install(FILES
  #  ${CMAKE_CURRENT_BINARY_DIR}/html/gdcm2vtk.html
  #  ${CMAKE_CURRENT_BINARY_DIR}/html/gdcmconv.html
  #  ${CMAKE_CURRENT_BINARY_DIR}/html/gdcmanon.html
  #  ${CMAKE_CURRENT_BINARY_DIR}/html/gdcmgendir.html
  #  ${CMAKE_CURRENT_BINARY_DIR}/html/gdcmdump.html
  #  ${CMAKE_CURRENT_BINARY_DIR}/html/gdcmimg.html
  #  ${CMAKE_CURRENT_BINARY_DIR}/html/gdcminfo.html
  #  ${CMAKE_CURRENT_BINARY_DIR}/html/gdcmpdf.html
  #  ${CMAKE_CURRENT_BINARY_DIR}/html/gdcmraw.html
  #  ${CMAKE_CURRENT_BINARY_DIR}/html/gdcmscanner.html
  #  ${CMAKE_CURRENT_BINARY_DIR}/html/gdcmtar.html
  #  ${CMAKE_CURRENT_BINARY_DIR}/html/gdcmviewer.html
  #  DESTINATION ${GDCM_INSTALL_DOC_DIR} COMPONENT DebugDevel
  #)
  #install(FILES
  #  ${CMAKE_CURRENT_BINARY_DIR}/man/man1/gdcm2vtk.1
  #  ${CMAKE_CURRENT_BINARY_DIR}/man/man1/gdcm2pnm.1
  #  ${CMAKE_CURRENT_BINARY_DIR}/man/man1/gdcmconv.1
  #  ${CMAKE_CURRENT_BINARY_DIR}/man/man1/gdcmpap3.1
  #  ${CMAKE_CURRENT_BINARY_DIR}/man/man1/gdcmxml.1
  #  ${CMAKE_CURRENT_BINARY_DIR}/man/man1/gdcmanon.1
  #  ${CMAKE_CURRENT_BINARY_DIR}/man/man1/gdcmgendir.1
  #  ${CMAKE_CURRENT_BINARY_DIR}/man/man1/gdcmdump.1
  #  ${CMAKE_CURRENT_BINARY_DIR}/man/man1/gdcmdiff.1
  #  ${CMAKE_CURRENT_BINARY_DIR}/man/man1/gdcmimg.1
  #  ${CMAKE_CURRENT_BINARY_DIR}/man/man1/gdcminfo.1
  #  ${CMAKE_CURRENT_BINARY_DIR}/man/man1/gdcmpdf.1
  #  ${CMAKE_CURRENT_BINARY_DIR}/man/man1/gdcmraw.1
  #  ${CMAKE_CURRENT_BINARY_DIR}/man/man1/gdcmscu.1
  #  ${CMAKE_CURRENT_BINARY_DIR}/man/man1/gdcmscanner.1
  #  ${CMAKE_CURRENT_BINARY_DIR}/man/man1/gdcmtar.1
  #  ${CMAKE_CURRENT_BINARY_DIR}/man/man1/gdcmviewer.1
  #  DESTINATION ${GDCM_INSTALL_MAN_DIR}/man1 COMPONENT DebugDevel
  #)
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION ${GDCM_INSTALL_DOC_DIR}
    PATTERN "*.md5" EXCLUDE
    PATTERN "*.dot" EXCLUDE
  )

  # http://lists.debian.org/debian-italian/2006/12/msg00878.html
  # man2txt : man man | col -b | less

  if(GDCM_USE_VTK)
    # Make this option a hidden option as vtkgdcm/doxygen stuff is a mess (copy/paste
    # from paraview). It always rerun -sigh-
    if(GDCM_VTK_DOCUMENTATION)
      set(BUILD_DOCUMENTATION 1)
    endif()
    subdirs(vtk)
  endif()
endif()

option(GDCM_BUILD_DOCBOOK_MANPAGES "Build man pages from XML docbook ?" ON)
mark_as_advanced(GDCM_BUILD_DOCBOOK_MANPAGES)
if(GDCM_BUILD_DOCBOOK_MANPAGES)
set(MANPAGES_XML
    gdcm2pnm
    gdcm2vtk
    gdcmanon
    gdcmconv
    gdcmdiff
    gdcmdump
    gdcmgendir
    gdcmimg
    gdcminfo
    gdcmpap3
    gdcmpdf
    gdcmraw
    gdcmscanner
    gdcmscu
    gdcmtar
    gdcmviewer
    gdcmxml
    )
find_package(LibXslt)
# need an XSLT 1.0 processor, use xsltproc, maybe add more implementations later
if(LIBXSLT_XSLTPROC_EXECUTABLE)
  set(XSLT_PROCESSOR ${LIBXSLT_XSLTPROC_EXECUTABLE})
# http://docbook.sourceforge.net/release/xsl/current/doc/manpages/man.output.quietly.html
  set(XSLT_PROCESSOR_ARG --param man.output.quietly 1)
  # User can change the behavior at cmake time:
  if(NOT DEFINED GDCM_MANPAGES_USE_NONET)
    set(GDCM_MANPAGES_USE_NONET FALSE)
    # By default I want nonet on Debian (docbook-xsl-ns)
    # on fedora you need to install docbook-style-xsl
    if(EXISTS /etc/xml/catalog)
      # https://cmake.org/pipermail/cmake/2007-May/014317.html
      file(READ /etc/xml/catalog xml_catalog_in)
      #<rewriteSystem systemIdStartString="http://docbook.sourceforge.net/release/xsl-ns/" rewritePrefix="file:///usr/share/sgml/docbook/xsl-ns-stylesheets-1.78.1"/>
      #<rewriteURI uriStartString="http://docbook.sourceforge.net/release/xsl-ns/" rewritePrefix="file:///usr/share/sgml/docbook/xsl-ns-stylesheets-1.78.1"/>
      string(REGEX REPLACE "\r?\n" ";" xml_catalog ${xml_catalog_in})

      set(DOCBOOK_REWRITE_PREFIX_FOUND FALSE)
      foreach(line ${xml_catalog})
        string(REGEX MATCH "^.*systemIdStartString=\"http://docbook.sourceforge.net/release/xsl-ns/\".*" out1 ${line})
        string(REGEX REPLACE "^.*systemIdStartString=\"http://docbook.sourceforge.net/release/xsl-ns/\".*\"([A-Za-z ]*)\".*" "\\1" output_variable ${line})
        if(out1)
          message(STATUS "Your docbook install was found here :${output_variable}")
          set(DOCBOOK_REWRITE_PREFIX_FOUND TRUE)
        endif()
      endforeach()
      # Need both the catalog and the docbook installation
      if(DOCBOOK_REWRITE_PREFIX_FOUND)
        set(GDCM_MANPAGES_USE_NONET TRUE)
      else()
        message(WARNING "Could not find the location of your docbook-xsl-ns installation. Please make sure your internet connection works. Or skip man pages generation: GDCM_BUILD_DOCBOOK_MANPAGES:BOOL=OFF.")
      endif()
    else()
      message(STATUS "Could not find /etc/xml/catalog")
    endif()
  endif()
  if(GDCM_MANPAGES_USE_NONET)
    list(APPEND XSLT_PROCESSOR_ARG --nonet)
  endif()
else()
  find_program(MSXSL_EXECUTABLE
    msxsl
    )
  mark_as_advanced(MSXSL_EXECUTABLE)
  if(MSXSL_EXECUTABLE)
    set(XSLT_PROCESSOR ${MSXSL_EXECUTABLE})
    set(XSLT_PROCESSOR_ARG "-pi")
# msxsl.exe -pi gdcmimg.xml
#
#Error occurred while executing stylesheet 'http://docbook.sourceforge.net/release/xsl-ns/current/manpages/docbook.xsl'.
#
#Code:   0x80004005
#Namespace 'http://exslt.org/common' does not contain any functions.
# -> http://stackoverflow.com/a/16605034/136285 ??
  endif()
endif()
  if(XSLT_PROCESSOR)
    configure_file(version.txt.in
      version.txt @ONLY)
    foreach(docbook ${MANPAGES_XML})
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${docbook}.1
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/man/${docbook}.xml ${CMAKE_CURRENT_BINARY_DIR}/${docbook}.xml
        COMMAND ${XSLT_PROCESSOR} ${XSLT_PROCESSOR_ARG} ${CMAKE_CURRENT_BINARY_DIR}/${docbook}.xml
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/man/${docbook}.xml
        )
    list(APPEND MANPAGES
      ${CMAKE_CURRENT_BINARY_DIR}/${docbook}.1
      )
  endforeach()
  add_custom_target(DOCBOOK_MANPAGES
    ALL
    DEPENDS ${MANPAGES}
    COMMENT  "docbook manpages"
    )
  install(FILES
    ${MANPAGES}
    DESTINATION ${GDCM_INSTALL_MAN_DIR}/man1 COMPONENT DebugDevel
  )
  else()
    message(WARNING "Cannot build man page from docbook (need an XSL processor)")
  endif()
endif() # GDCM_BUILD_DOCBOOK_MANPAGES
