Freetalk
--------

[![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/GNUFreetalk/freetalk?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

Freetalk is a console based Jabber client. It features a readline
interface with completion of buddy names, commands, and even ordinary
English words! Freetalk is extensible, configurable, and scriptable
through a Guile interface.

GNU Freetalk is distributed under the GNU General Public License,
version 3 or later --- see the file COPYING for details.

Usage
--------
To start with you can use Freetalk by just running 'freetalk' and
then doing `/login` to specify your Jabber account and password. To
avoid having to do this every time, you can put your account details
in a file called `~/.freetalk/freetalk.scm`. Look at `examples/freetalk.scm`
to know how to do this.

There is comprehensive documentation in Info format in the doc/ directory,
which you should be able to read by saying,

       $ info freetalk

Queries
--------
If you face any problems compiling or using Freetalk, or if you have
any feedback/developer questions, feel free to post a message to
`harsha (at) harshavardhana.net` or open a github issue.
