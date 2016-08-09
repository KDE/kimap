# KIMAP #

This library provides a job-based API for interacting with an IMAP4rev1 server.
It manages connections, encryption and parameter quoting and encoding, but
otherwise provides quite a low-level interface to the protocol.  This library
does not implement an IMAP client; it merely makes it easier to do so.

Users should be familiar with [RFC 3501](http://www.apps.ietf.org/rfc/rfc3501.html "IMAP 4rev1")
as well as [other related RFCs](http://www.iana.org/assignments/imap4-capabilities)
although the library hides some of the nastier details like the encoding and quoting of
strings.
