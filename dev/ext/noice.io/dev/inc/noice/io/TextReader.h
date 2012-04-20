/*-------------------------------------------------------------------------
File    : $Archive: TextReader.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-11-02, 15:50
Descr   : Implements a class that reads text lines from a stream

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
- Enhance to support simple reading of bytes as well as lines
</pre>


\History
- 02.11.09, FKling, Implementation

---------------------------------------------------------------------------*/
#pragma once

#include "noice/io/io.h"

namespace noice
{
	namespace io
	{
		class TextReader
		{
		protected:
			long sz;
			long pos, endpos;
			char *buffer;
			bool bHaveData;
			IStream *pStream;
		protected:
			char *FindLine();
			void MoreData();

			static const int kTextReader_ChunkSize;
		public:
			TextReader(IStream *pStream);
			virtual ~TextReader();

			bool Close();
			char *ReadLine();
			char *ReadLine(char *pDest, int nMax);

			int Read(char *pDest, int nBytes);
		};

	}
}