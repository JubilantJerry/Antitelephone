call doxygen doxy.cfg
call python doxygen2qtcreator/doxygen2qtcreator.py html
call J:\AppTools\QTCreator\5.8\msvc2013_64\bin\qhelpgenerator html/index.qhp -o documentation.qch