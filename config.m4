PHP_ARG_WITH(v8, whether to enable Google v8 support,
[  --with-v8                Enable Google v8 support])


if test "$PHP_V8" != "no"; then


  if test "$PHP_V8" != "yes"; then
    if test -r "$PHP_V8/include/v8/v8.h"; then
      PHP_LIBV8_DIR="$PHP_V8"
      PHP_LIBV8_INCDIR="$PHP_V8/include/v8"
    elif test -r "$PHP_V8/include/v8.h"; then
      PHP_LIBV8_DIR="$PHP_V8"
      PHP_LIBV8_INCDIR="$PHP_V8/include"
    else
      AC_MSG_ERROR([Can't find libv8 headers under "$PHP_V8"])
    fi
  else
    PHP_LIBV8_DIR="no"
    for i in /usr /usr/local; do
      if test -r "$i/include/v8/v8.h"; then
        PHP_LIBV8_DIR=$i
        PHP_LIBV8_INCDIR="$i/include/v8"
        break
      elif test -r "$i/include/v8.h"; then
        PHP_LIBV8_DIR=$i
        PHP_LIBV8_INCDIR="$i/include"
        break
      fi
    done
  fi

  AC_MSG_CHECKING([for libv8 location])
  if test "$PHP_LIBV8_DIR" = "no"; then
    AC_MSG_ERROR([v8 support requires libv8. Use --with-v8=<DIR> to specify the prefix where libv8 headers and library are located])
  else
    AC_MSG_RESULT([$PHP_LIBV8_DIR])
    PHP_REQUIRE_CXX()
    PHP_ADD_LIBRARY(stdc++, 1, V8_SHARED_LIBADD)

    AC_MSG_RESULT([$PHP_LIBV8_INCDIR])

    PHP_ADD_INCLUDE($PHP_LIBV8_INCDIR)
    PHP_ADD_LIBRARY_WITH_PATH(v8, $PHP_LIBV8_DIR, V8_SHARED_LIBADD)

    PHP_SUBST(V8_SHARED_LIBADD)
    PHP_NEW_EXTENSION(v8, [v8.cpp v8_v8context.cpp v8_conv.cpp], $ext_shared, [], [])

  fi

fi

