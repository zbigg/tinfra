dnl
dnl makefoo
dnl

AC_DEFUN([AC_MAKEFOO],
[
    #echo "ac_top_srcdir = $ac_top_srcdir"
    #echo "top_srcdir = $top_srcdir"
    #echo "srcdir = $srcdir"
    
    AC_ARG_WITH(makefoo-dir,[--with-makefoo-dir=DIR   Where makefoo is installed (mandatory if not standard)],
            [
                makefoo_dir="$withval"
                makefoo_main="$withval"/main.mk
                AC_MSG_NOTICE([using custom makefoo $makefoo_main ])
            ]
            , makefoo_dir="")
            
    #AC_MSG_CHECKING(for makefoo path)
    
    if test x$makefoo_dir = x ; then
        AC_MSG_CHECKING([for makefoo path with pkg_config])
        makefoo_dir=`pkg-config --variable=MAKEFOO_dir makefoo 2>/dev/null`
        if test -f $makefoo_dir/defs.mk ; then
            makefoo_main="$makefoo_dir/main.mk"
            AC_MSG_RESULT([$makefoo_main])
        else
            AC_MSG_RESULT([not found])
        fi
    fi
    if test x$makefoo_dir = x ; then
        AC_MSG_CHECKING([for makefoo in predefined folders])
        for DIR in $srcdir/.makefoo $srcdir/makefoo $srcdir $HOME/share/makefoo /usr/local/share/makefoo /usr/share/makefoo; do
            if test -f $DIR/defs.mk ; then
                makefoo_dir="$DIR"
                makefoo_main="$DIR/main.mk"
                # note, not sure if it's bash
                # or posix feature
                AC_MSG_RESULT([$makefoo_main])
                break
            fi
        done
        if test -f $srcdir/makefoo_amalgamation.mk  ; then
            makefoo_dir="$DIR"
            makefoo_main="$DIR/makefoo_amalgamation.mk"
            AC_MSG_RESULT([$makefoo_main])
            break
        fi
    fi
    
    if test x$makefoo_main = x ; then
        # download from github support
        makefoo_version="0.0.1"
        makefoo_download_url="https://github.com/zbigg/makefoo/archive/makefoo-${makefoo_version}.zip"
        makefoo_zip_file=".makefoo-${makefoo_version}.zip"
        
        if type unzip > /dev/null 2>/dev/null ; then
            if ! test -f $makefoo_zip_file ; then
                if type wget >/dev/null 2>/dev/null ; then
                    AC_MSG_CHECKING([downloading makefoo from $makefoo_download_url])
                    if ! wget --no-check-certificate $makefoo_download_url -O "$makefoo_zip_file"; then
                        AC_MSG_RESULT([failed to download makefoo])
                    fi
                elif type curl >/dev/null 2>/dev/null ; then
                    AC_MSG_CHECKING([downloading makefoo from $makefoo_download_url])
                    if ! curl $makefoo_download_url > "$makefoo_zip_file"; then
                        AC_MSG_RESULT([failed to download makefoo])
                    fi
                fi
            else
                AC_MSG_CHECKING([using already downloaded $makefoo_zip_file])
            fi
            if test -f "$makefoo_zip_file" ; then
                rm -rf .makefoo
                if unzip -q "$makefoo_zip_file" ; then
                    mv makefoo-makefoo-"$makefoo_version" .makefoo
                    if test -f .makefoo/main.mk ; then
                        makefoo_dir=".makefoo"
                        makefoo_main="$makefoo_dir/main.mk"
                        AC_MSG_RESULT([$makefoo_main])
                    else
                        AC_MSG_RESULT([problem with downloaded archive in .makefoo, no main.mk found])
                    fi
                fi
            fi
        fi
    fi
    
    if test x$makefoo_main = x ; then
        #AC_MSG_RESULT([not found])
        AC_MSG_ERROR([makefoo not found in default locations, please try --with-makefoo-dir=FOLDER option])
    fi
    
    MAKEFOO=${makefoo_main}
    
    # we require canonical host to configure makefoo
    AC_CANONICAL_HOST
    #
    # now generate makefoo_configured_defs
    #
    #echo "host = $host"
    AC_MSG_NOTICE([generating makefoo configuration: makefoo_configured_defs.mk])
    
    rm -rf makefoo_configured_defs.mk
    MAKEFOO_dir=${makefoo_dir} target_arch=$host ${makefoo_dir}/configure.sh > makefoo_configured_defs.mk
    
    AC_MSG_NOTICE([makefoo config:])
    cat makefoo_configured_defs.mk
    AC_SUBST(MAKEFOO)
])

# jedit: :tabSize=8:mode=shellscript:
