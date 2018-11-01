find . -name *.h | xargs dos2unix
find . -name *.c | xargs dos2unix
find . -name *.cc | xargs dos2unix
find . -name *.cpp | xargs dos2unix
find . -name *.h | xargs clang-format -style="{BasedOnStyle: Google, ColumnLimit: 100}" -i
find . -name *.c | xargs clang-format -style="{BasedOnStyle: Google, ColumnLimit: 100}" -i
find . -name *.cc | xargs clang-format -style="{BasedOnStyle: Google, ColumnLimit: 100}" -i
find . -name *.cpp | xargs clang-format -style="{BasedOnStyle: Google, ColumnLimit: 100}" -i
