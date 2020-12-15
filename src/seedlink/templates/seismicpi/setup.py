import os

'''
Plugin handler for the seismicpi plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # Check and set defaults
    try: seedlink.param('sources.seismicpi.port_path_hint')
    except: seedlink.setParam('sources.seismicpi.port_path_hint', '/dev/ttyS0')

    try: seedlink.param('sources.seismicpi.allow_set_interface_attribs')
    except: seedlink.setParam('sources.seismicpi.allow_set_interface_attribs', 1)

    try: seedlink.param('sources.seismicpi.mswrite_header_sample_rate')
    except: seedlink.setParam('sources.seismicpi.mswrite_header_sample_rate', '1')

    try: seedlink.param('sources.seismicpi.mswrite_data_encoding_type')
    except: seedlink.setParam('sources.seismicpi.mswrite_data_encoding_type', 'STEIM2')

    try:  seedlink.param('sources.seismicpi.channel_prefix')
    except: seedlink.setParam('sources.seismicpi.channel_prefix', 'BH')

    try:  seedlink.param('sources.seismicpi.locationcode_prefix')
    except: seedlink.setParam('sources.seismicpi.locationcode_prefix', '00')

    try: seedlink.param('sources.seismicpi.component')
    except: seedlink.setParam('sources.seismicpi.component', 'Z')

    try: seedlink.param('sources.seismicpi.do_settings_pihat')
    except: seedlink.setParam('sources.seismicpi.do_settings_pihat', '1')

    try: seedlink.param('sources.seismicpi.nominal_sample_rate')
    except: seedlink.setParam('sources.seismicpi.nominal_sample_rate', '32')

    try: seedlink.param('sources.seismicpi.nominal_gain')
    except: seedlink.setParam('sources.seismicpi.nominal_gain', '1')

    try: seedlink.param('sources.seismicpi.single_multi')
    except: seedlink.setParam('sources.seismicpi.single_multi', '1')

    try: seedlink.param('sources.seismicpi.select_source')
    except: seedlink.setParam('sources.seismicpi.select_source', '1')

    tag = seedlink.net + "." + seedlink.sta

    fd = open(os.path.join(seedlink.config_dir, tag + ".ini"), "w")
    fd.write(seedlink._process_template('seismicpi.prop.tpl', 'seismicpi'))
    fd.close()

    return tag

  # Flush does nothing
  def flush(self, seedlink):
    pass
