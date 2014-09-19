import os

'''
Plugin handler for the minilogger plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # Check and set defaults
    try: seedlink.param('sources.minilogger.port_path_hint')
    except: seedlink.setParam('sources.minilogger.port_path_hint', '/dev/ttyACM0')

    try: seedlink.param('sources.minilogger.allow_set_interface_attribs')
    except: seedlink.setParam('sources.minilogger.allow_set_interface_attribs', 1)

    try: seedlink.param('sources.minilogger.mswrite_header_sample_rate')
    except: seedlink.setParam('sources.minilogger.mswrite_header_sample_rate', '-1')

    try: seedlink.param('sources.minilogger.mswrite_data_encoding_type')
    except: seedlink.setParam('sources.minilogger.mswrite_data_encoding_type', 'STEIM2')

    try:  seedlink.param('sources.minilogger.channel_prefix')
    except: seedlink.setParam('sources.minilogger.channel_prefix', 'SH')

    try: seedlink.param('sources.minilogger.component')
    except: seedlink.setParam('sources.minilogger.component', 'Z')

    try: seedlink.param('sources.minilogger.do_settings_sep064')
    except: seedlink.setParam('sources.minilogger.do_settings_sep064', '1')

    try: seedlink.param('sources.minilogger.nominal_sample_rate')
    except: seedlink.setParam('sources.minilogger.nominal_sample_rate', '80')

    try: seedlink.param('sources.minilogger.nominal_gain')
    except: seedlink.setParam('sources.minilogger.nominal_gain', '4')

    tag = seedlink.net + "." + seedlink.sta + "." + seedlink.param('sources.minilogger.channel_prefix') + seedlink.param('sources.minilogger.component')

    fd = open(os.path.join(seedlink.config_dir, tag + ".prop"), "w")
    fd.write(seedlink._process_template('minilogger.prop.tpl', 'minilogger'))
    fd.close()

    return tag

  # Flush does nothing
  def flush(self, seedlink):
    pass
