_seiscomp()
{
	local cur prev opts base
	COMPREPLY=()
	bin=${COMP_WORDS[0]}
	cur="${COMP_WORDS[COMP_CWORD]}"
	prev="${COMP_WORDS[COMP_CWORD-1]}"

	#
	#  The basic options we'll complete.
	#
	local sc3Commands="install-deps setup shell enable disable start stop restart check status list exec update-config alias print help"

	#
	#  Complete the arguments to some of the basic commands.
	#
	case "${prev}" in
		alias)
			COMPREPLY=( $(compgen -W "create remove" -- ${cur}) )
			return 0
		;;
		exec)
			COMPREPLY=( $(compgen -W "$(ls $(eval $bin exec env|grep SEISCOMP_ROOT|cut -d "=" -f2)/bin)" -- ${cur}) )
			return 0
		;;
		help)
			COMPREPLY=( $(compgen -W "${sc3Commands}" -- ${cur}) )
			return 0
		;;
		install-deps)
			# search available scripts in OS dependent dependency folder
			if  command -v lsb_release > /dev/null; then
				local sc3Root=$(eval $bin exec env|grep SEISCOMP_ROOT|cut -d "=" -f2)
				local sc3Deps=$(ls $sc3Root/share/deps/$(lsb_release -sir | tr "\n" "/" | tr '[:upper:]' '[:lower:]'))
				local sc3Deps=$(basename -s .sh -a ${sc3Deps[@]//install-/})
				COMPREPLY=( $(compgen -W "${sc3Deps}" -- ${cur}) )
			fi
			return 0
		;;
		list)
			COMPREPLY=( $(compgen -W "modules aliases enabled disabled" -- ${cur}) )
			return 0
		;;
		print)
			COMPREPLY=( $(compgen -W "crontab env" -- ${cur}) )
			return 0
		;;
		enable|disable|start|stop|restart|check|status|update-config)
			COMPREPLY=( $(compgen -W "$(eval $bin list modules | cut -d " " -f1)" -- ${cur}) )
			return 0
		;;
		$bin)
			COMPREPLY=( $(compgen -W "$sc3Commands" -- $cur) )
			return 0
		;;
	esac
}

complete -F _seiscomp seiscomp
