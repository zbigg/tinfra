# TODO: rewrite as script, makefile is not needed here
LOCAL_DB=tinfra.mtn
TAILOR="tailor -v --source-repository $(mtn au get_option database)"
SVNROOT=https://tinfra.googlecode.com/svn/trunk/

verbose()
{
	echo "$@"
	"$@"
}
run_tailor()
{
	project=$1
	branch=$2
	rootdir="${project}-rootdir"
	if [ ! -d $rootdir/svn-side ] ; then
		verbose svn co $SVNROOT/${project} $rootdir/svn-side 
	fi
	
	if [ ! -d $rootdir/mtn-side ] ; then
		verbose mtn co -b "${branch}" ${rootdir}/mtn-side 
	fi
	
	verbose $TAILOR -c ${project}.tailor
}

set -e

run_tailor tinfra pl.reddix.tinfra
run_tailor tinfra-xml pl.reddix.tinfra-xml
run_tailor tinfra-regexp pl.reddix.tinfra-regexp
run_tailor tinfra-ssh pl.reddix.tinfra-ssh
run_tailor tinfra-support pl.reddix.tinfra

