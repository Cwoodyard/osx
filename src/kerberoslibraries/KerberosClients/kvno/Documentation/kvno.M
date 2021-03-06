.\" Copyright (C) 1998 by the FundsXpress, INC.
.\"
.\" All rights reserved.
.\"
.\" Export of this software from the United States of America may require
.\" a specific license from the United States Government.  It is the
.\" responsibility of any person or organization contemplating export to
.\" obtain such a license before exporting.
.\"
.\" WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
.\" distribute this software and its documentation for any purpose and
.\" without fee is hereby granted, provided that the above copyright
.\" notice appear in all copies and that both that copyright notice and
.\" this permission notice appear in supporting documentation, and that
.\" the name of FundsXpress. not be used in advertising or publicity pertaining
.\" to distribution of the software without specific, written prior
.\" permission.  FundsXpress makes no representations about the suitability of
.\" this software for any purpose.  It is provided "as is" without express
.\" or implied warranty.
.\"
.\" THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
.\" IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
.\" WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
.\"
.\" "
.TH KVNO 1
.SH NAME
kvno \- print key version numbers of Kerberos principals
.SH SYNOPSIS
\fBkvno\fP [\fB\-h\fP] | [\fB\-q\fP] [\fB\-c ccache\fP] [\fB\-e etype\fP] 
[\fB\-k keytab\fP] \fBservice1\fP \fBservice2\fP \fB...\fP
.br
.SH DESCRIPTION
.I Kvno
acquires a service ticket for each of the specified Kerberos principals 
and prints out the key version numbers of each.
.SH OPTIONS
.TP
.B \-h
prints a usage statement and exits.
.TP
.B \-q
suppress printing.
.TP
.B \-c ccache
specifies the name of a credentials cache to use.  If this option is not
specified then the default credentials cache is used.
.TP
.B \-e enctype
specifies the enctype which will be requested for the session key of
all the services named on the command line.  This is useful in certain
backward compatibility situations.
.TP
.B \-k keytab
specifies the name of a keytab which will be used to validate the service 
ticket.
.SH SEE ALSO
kinit(1), kdestroy(1)
