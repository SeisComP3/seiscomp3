
try:
    fd = file("/srv/www/htdocs/index.html")
    _head = ""
    buf = fd.readline()
    while buf:
        if buf.startswith("<!-- end header -->"):
            break
        
        _head += buf
        buf = fd.readline()

except IOError:
    _head="""<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"><html><head><title>webdc</title></head><body>"""

query_head=_head

query_chunk1="""<hr style="width: 100%; height: 2px;">
<br>
<table
style="width: 85%; text-align: left; margin-left: auto; margin-right: auto;">
<tbody>
<tr>
<td style="vertical-align: middle; text-align: left;">
<h3><font><font face="Arial, Helvetica, sans-serif">Get
information about or waveform data from a selected "real" or a
selfdefined "virtual" network...</font></font></h3>
</td>
</tr>
</tbody>
</table>
<div style="margin-left: 80px;">

<h3><font face="Arial, Helvetica, sans-serif"><font
face="Arial, Helvetica, sans-serif">Primary station selection:<br>
</font></font></h3>
</div>
<table
style="width: 85%; height: 60px; text-align: left; margin-left: auto; margin-right: auto;">
<tbody>
<tr>
<td style="text-align: center;">
<form name="typesel"><b><font face="Arial, Helvetica, sans-serif"><b>1. Select group of networks:
<select
onchange="location.href=this.form.typesel.options[selectedIndex].value;"
name="typesel" size="1">"""

query_chunk2="""</select>
</b> </font></b><br>
</form>
</td>
</tr>
</tbody>
</table>
<br>
<table
style="width: 85%; height: 60px; text-align: left; margin-left: auto; margin-right: auto;">
<tbody>
<tr>
<td
style="vertical-align: middle; width: 25%; text-align: center;">
<form name="netform"> <b><font
face="Arial, Helvetica, sans-serif"><b>2. Select network code:
<select
onchange="location.href=this.form.netsel.options[selectedIndex].value;"
name="netsel" size="1">"""

query_chunk3="""</select><br></b></b>
(* internal code for temporary networks)
</font> </form>

</td>
<td
style="vertical-align: middle; width: 25%; text-align: center;">
<form name="statform"> <b><font
face="Arial, Helvetica, sans-serif"><b>3. Select station
code:
<select
onchange="location.href=this.form.statsel.options[selectedIndex].value;"
name="statsel" size="1">"""

query_chunk4="""</select>

</b> </font></b> </form>
</td>

</tr>
</tbody>
</table>

<div style="margin-left: 80px;">
<font face="Arial, Helvetica, sans-serif"><h3>
4. Optional stations selections:
</b> (strongly recommended for virtual networks)</h3></font></div>
<form action="select"
method="get"> <font face="Arial, Helvetica, sans-serif"> <font
face="Arial, Helvetica, sans-serif">"""

query_tail="""<table
style="width: 85%; height: 60px; text-align: left; margin-left: auto; margin-right: auto;">
<tbody>
<tr>

<center><b>Sensor type:
<select name="sensor" size="1">
<option value="all" selected>all</option><option value="VBB+BB" >VBB+BB</option><option value="VBB" >VBB</option><option value="BB" >BB</option><option value="SP" >SP</option><option value="SM" >SM</option><option value="OBS" >OBS</option>

</select>
Location code: <input type="text" name="loc_id" size="2" value="*">
Stream: <input type="text" name="stream" size="3" value="*">

</b></font></b><b><font face="Arial, Helvetica, sans-serif"><b><br>
</b></font></b></center> </tr><tr>
        <td
 style="vertical-align: middle; width: 50%; text-align: center;">
        <table style="width: 100%; text-align: left;">
          <tbody>
            <tr>
              <td style="vertical-align: middle; text-align: center;"><font
 face="Arial, Helvetica, sans-serif"><b>Geographical region: <input type="text"
 name="latmin" value="-90" size="3"> &lt;= Lat.
&lt;= <input type="text" name="latmax" value="90" size="3">&nbsp;| <input
 type="text" name="lonmin" value="-180" size="3"> &lt;= Lon.

&lt;= <input type="text" name="lonmax" value="180" size="3"></b></font>
              </td>
            </tr>
            <tr>
              <td style="vertical-align: middle; text-align: center;"><font
 face="Arial, Helvetica, sans-serif"><b>Operation time period of interest: <input type="text"
 name="start_date" value="1990-01-01" size="10"> &lt;= Date
&lt;= <input type="text" name="end_date" value="2030-12-31" size="10"></b></font>
              </td>
            </tr>

          </tbody>
        </table>
        </td>
</tr>
</tbody>
</table>
</font> </font>

<br><center><font face="Arial, Helvetica, sans-serif"><b><input type="submit"
src="request_files/request.html" value="Submit"
name="submit"></b></font></b></font></center>
</form>
<hr style="width: 100%; height: 2px;">
</b><font><br><br></html>"""

select_head=_head

select_chunk1="""<hr style="width: 100%; height: 2px;">
<br>
<table
style="width: 85%; text-align: left; margin-left: auto; margin-right: auto;">
<tbody>
<tr>
<td style="vertical-align: middle; text-align: left;">
<h3><font><font face="Arial, Helvetica, sans-serif">
Select streams:
</font></font></h3>
</td>
</tr>
</tbody>
</table>
<form method="get" action="submit">
<center>
<table>
<tbody>"""

select_chunk2="""</tbody>
</table>
</center>"""

select_chunk3="""<br><HR WIDTH="100%">
<center>"""

select_tail="""</center>
<br><CENTER><FONT SIZE=+0>E-Mail (no dummy!)*: </FONT><INPUT type="text" value="" size=15 maxlength=60 min=6 name="user"></CENTER>
<br><CENTER><FONT SIZE=+0>Full name (no dummy!)*: </FONT><INPUT type="text" value="" size=15 maxlength=60 min=6 name="name"></CENTER>
<br><CENTER><FONT SIZE=+0>Name of institute*: </FONT><INPUT type="text" value="" size=20 maxlength=60 min=6 name="inst"></CENTER>
<br><CENTER><FONT SIZE=+0>Snail mail address: </FONT><INPUT type="text" value="" size=31 maxlength=60 min=6 name="mail"></CENTER>
<br><CENTER><FONT SIZE=+0>Primary ArcLink node:
<select name="arclink" size="1">
<option value="localhost:18001">localhost</option>
</select>
<br><center>* Mandatory input.<br>

<hr style="width: 100%; height: 2px;">

<h3><font><font face="Arial, Helvetica, sans-serif">
Select format:
</font></font></h3>

<center><font face="Arial, Helvetica, sans-serif">
<table style="text-align: left; width: 85%; height: 30px;">
<tbody>
<tr>
<td style="text-align: center; vertical-align: middle;"><b>
Mini-SEED<input type="radio" name="format" value="MSEED" checked="checked">|&nbsp;
Full SEED <input type="radio" name="format" value="FSEED">|&nbsp;
Dataless SEED <input type="radio" name="format" value="DSEED">|&nbsp;
Inventory (XML) <input type="radio" name="format" value="INV">
</b></font></b></td>
</tr>
<tr>
<td style="text-align: center; vertical-align: middle;"><b>
<input type="checkbox" name="resp_dict" value="true" checked="checked">Use response dictionary (SEED blockettes 4x)
</b></font></b></td>
</tr>
</tbody>
</table>
<h3><font><font face="Arial, Helvetica, sans-serif">
Select compression:
</font></font></h3>

<center><font face="Arial, Helvetica, sans-serif">
<table style="text-align: left; width: 85%; height: 30px;">
<tbody>
<tr>
<td style="text-align: center; vertical-align: middle;"><b>
bzip2<input type="radio" name="compression" value="bzip2" checked="checked">|&nbsp;
none <input type="radio" name="compression" value="none">
</b></font></b></td>
</tr>
</tbody>
</table>
<br>
<font face="Arial, Helvetica, sans-serif"><b><input type="submit"
src="request_files/request.html" value="Submit"
name="submit"></b></font></b></font></center>
</form>
<hr style="width: 100%; height: 2px;">
</html>"""

submit_head=_head + """<hr style="width: 100%; height: 2px;">
<br>
<table
style="width: 85%; text-align: left; margin-left: auto; margin-right: auto;">
<tbody>
</tbody>
</table>"""

submit_tail="""</html>"""

status_head=_head+"""<hr style="width: 100%; height: 2px;">
<br>
<table
style="width: 85%; text-align: left; margin-left: auto; margin-right: auto;">
<tbody>
</tbody>
</table>"""

status_tail="""</html>"""

