scsendjournal allows to manipulate event parameters by sending journals to the
messaging system. The manipulation allows to:

a. Create new events
#. Modify event parameters
#. Control the association of origins to events

Synopsis
========

scsendjournal [opts] {objectID} {action} [parameters]

.. _scsendjournal-options:

Actions
=======

There are specific actions for handling non-events and events.

None-event specific actions
---------------------------

a. EvNewEvent: create a new event from origin in the provided file

Origin association
------------------

a. EvGrabOrg: grab origin and move the origin to the event with the given eventID.
   If the origins is already associated to another event, remove this reference.
#. EvMerge: merge events into one event
#. EvSplitOrg: split origins to 2 events

Event parameters
----------------

a. EvName: set event Name
#. EvOpComment: set event operator's comment
#. EvPrefFocMecID: set event preferred focal mechanism
#. EvPrefMagType: set preferred magnitude type
#. EvPrefMw: set Mw from focal mechanism as preferred magnitude
#. EvPrefOrgAutomatic: set the preferred mode to *automatic* corresponding to *unfix* in scolv
#. EvPrefOrgEvalMode: set preferred origin by evaluation mode
#. EvPrefOrgID: set preferred origin by ID
#. EvType: set event type
#. EvTypeCertainty: set event type certainty

Examples
========

#. **EvMerge:** Merge all origins from the source event with eventID *eventS* into the target
   event with eventID *eventT*. Remove event *eventS*. Apply the action in message
   system on *host*:

   .. code-block:: sh

      scsendjournal -H {host} {eventT} EvMerge {eventS}
