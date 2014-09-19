
#
# This is scream2ew's parameter file

#  Basic Earthworm setup:
#
MyModuleId         MOD_SCREAM2EW  # module id for this instance of scream2ew 
RingName           WAVE_RING      # shared memory ring for input/output
LogFile            1              # 0 to turn off disk log file; if 1, do log.
                                  # if 2, log to module log but not stderr/stdout
Verbose            1              # 1=> log every packet. 0=> don't
HeartBeatInterval  30             # seconds between heartbeats
# SleepInterval      100          # Ms to sleep during each loop (Ignored, provided for compatability)
# Verbose	     1		  # Enable verbosity (Ignored, provided for compatability)
#
#
#
PortNumber         1567
Server             192.168.42.46  # If specified name or IP-number this module
                                  # will run in TCP-mode and requests data from
                                  # a screem server on the given PortNumber.
                                  # If unspecified this module will run in
                                  # UDP-mode and listen to all broadcast
                                  # messages on the specified PortNumber.
#
#
#

ChanInfo "D850	EKA12	FISH SOUP1	BHZ	1"
ChanInfo "D850	EKA22	FISH SOUP2	BHZ	2"
ChanInfo "D850	EKA32	FISH SOUP3	BHZ	3"
ChanInfo "D850	EKA42	FISH SOUP4	BHZ	4"
ChanInfo "D850	EKA62	FISH SOUP5	BHZ	5"
ChanInfo "D850	EKA72	FISH SOUP6	BHZ	6"
ChanInfo "D850	EKA82	FISH SOUP7	BHZ	7"
ChanInfo "D851	EKA92	FISH SOUP8	BHZ	8"
ChanInfo "D851	EKAA2	FISH SOUP9	BHZ	9"
ChanInfo "D851	EKAB2	FISH SOUP10	BHZ	10"
ChanInfo "D851	EKAC2	FISH SOUP11	BHZ	11"
ChanInfo "D851	EKAD2	FISH SOUP12	BHZ	12"
ChanInfo "D851	EKAE2	FISH SOUP13	BHZ	13"
ChanInfo "D851	EKAF2	FISH SOUP14	BHZ	14"
ChanInfo "D851	EKAG2	FISH SOUP15	BHZ	15"
ChanInfo "D852	EKAH2	FISH SOUP16	BHZ	16"
ChanInfo "D852	EKAI2	FISH SOUP17	BHZ	17"
ChanInfo "D852	EKAJ2	FISH SOUP18	BHZ	18"
ChanInfo "D852	EKAK2	FISH SOUP19	BHZ	19"
ChanInfo "D852	EKAL2	FISH SOUP20	BHZ	20"
ChanInfo "D852	EKAM2	FISH SOUP21	BHZ	21"
ChanInfo "D852	EKAN2	FISH SOUP22	BHZ	22"
ChanInfo "D852	EKAO2	FISH SOUP23	BHZ	23"
ChanInfo "GURALP	3661E2	FISH SOUP24	BHZ	24"
ChanInfo "GURALP	3661E6	FISH SOUP25	BHZ	25"
ChanInfo "GURALP	3661M8	FISH SOUP26	BHZ	26"
ChanInfo "GURALP	3661M9	FISH SOUP27	BHZ	27"
ChanInfo "GURALP	3661MA	FISH SOUP28	BHZ	28"
ChanInfo "GURALP	3661ME	FISH SOUP29	BHZ	29"
ChanInfo "GURALP	3661N2	FISH SOUP30	BHZ	30"
ChanInfo "GURALP	3661N6	FISH SOUP31	BHZ	31"
ChanInfo "GURALP	3661Z2	FISH SOUP32	BHZ	32"
ChanInfo "GURALP	3661Z6	FISH SOUP33	BHZ	33"
ChanInfo "GURALP	DIALE4	FISH SOUP34	BHZ	34"
ChanInfo "GURALP	DIALE6	FISH SOUP35	BHZ	35"
ChanInfo "GURALP	DIALM8	FISH SOUP36	BHZ	36"
ChanInfo "GURALP	DIALM9	FISH SOUP37	BHZ	37"
ChanInfo "GURALP	DIALMA	FISH SOUP38	BHZ	38"
ChanInfo "GURALP	DIALME	FISH SOUP39	BHZ	39"
ChanInfo "GURALP	DIALN4	FISH SOUP40	BHZ	40"
ChanInfo "GURALP	DIALN6	FISH SOUP41	BHZ	41"
ChanInfo "GURALP	DIALZ4	FISH SOUP42	BHZ	42"
ChanInfo "GURALP	DIALZ6	FISH SOUP43	BHZ	43"
ChanInfo "WO1242	3442E2	FISH SOUP44	BHZ	44"
ChanInfo "WO1242	3442E6	FISH SOUP45	BHZ	45"
ChanInfo "WO1242	3442N2	FISH SOUP46	BHZ	46"
ChanInfo "WO1242	3442N6	FISH SOUP47	BHZ	47"
ChanInfo "WO1242	3442Z2	FISH SOUP48	BHZ	48"
ChanInfo "WO1242	3442Z6	FISH SOUP49	BHZ	49"
ChanInfo "WO1343	4998E2	FISH SOUP50	BHZ	50"
ChanInfo "WO1343	4998E4	FISH SOUP51	BHZ	51"
ChanInfo "WO1343	4998E6	FISH SOUP52	BHZ	52"
ChanInfo "WO1343	4998M8	FISH SOUP53	BHZ	53"
ChanInfo "WO1343	4998M9	FISH SOUP54	BHZ	54"
ChanInfo "WO1343	4998MA	FISH SOUP55	BHZ	55"
ChanInfo "WO1343	4998ME	FISH SOUP56	BHZ	56"
ChanInfo "WO1343	4998N2	FISH SOUP57	BHZ	57"
ChanInfo "WO1343	4998N4	FISH SOUP58	BHZ	58"
ChanInfo "WO1343	4998N6	FISH SOUP59	BHZ	59"
ChanInfo "WO1343	4998Z2	FISH SOUP60	BHZ	60"
ChanInfo "WO1343	4998Z4	FISH SOUP61	BHZ	61"
ChanInfo "WO1343	4998Z6	FISH SOUP62	BHZ	62"
