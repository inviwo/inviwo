string(REPLACE "\"" "" qhp ${QHP_FILE})
string(REPLACE "\"" "" stripped ${STRIPPED_FILE})

file(STRINGS ${qhp} seclines REGEX "<section.*ref=\"docpage-.*./>")
string(REGEX REPLACE ";" "\n" newSecLines "${seclines}")

file(STRINGS ${qhp} docFilelines REGEX "<file>docpage-.*html</file>")
string(REGEX REPLACE ";" "\n" newDocFilelines "${docFilelines}")

file(STRINGS ${qhp} filelines REGEX "<file>.*\\.(css|png|jpeg|jpg)</file>")
string(REGEX REPLACE ";" "\n" newFilelines "${filelines}")

file(STRINGS ${qhp} namespacelines REGEX "<namespace>.*</namespace>")
string(REGEX REPLACE ";" "\n" newNamespacelines "${namespacelines}")

set(file "\
<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<QtHelpProject version=\"1.0\">
${newNamespacelines}
  <virtualFolder>doc</virtualFolder>
  <filterSection>
    <filterAttribute>doxygen</filterAttribute>
    <toc>
${newSecLines}
    </toc>
    <keywords>
    </keywords>
    <files>
${newDocFilelines}
${newFilelines}
    </files>
  </filterSection>
</QtHelpProject>
")

file(WRITE ${stripped} ${file})