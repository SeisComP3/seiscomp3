Running SeisComP3 causes many database accesses for writing. Anytime a new
event has been created a new row will be inserted in the database table.
When the same event is updated the row in the database table is going to be
changed as well. The information about the history of the event is lost because
the database contains only the current event attributes. scevtlog saves the
event history into files. While scevtlog is running it keeps the track of all
event updates and stores this information in a directory that can be analyzed
at anytime. The stored information is written as plain text in an easily
readable format. Additionally scevtlog maintains an event summary file for an
overview of the event history.
