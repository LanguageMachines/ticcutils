/*
  Copyright (c) 2006 - 2024
  CLST  - Radboud University
  ILK   - Tilburg University

  This file is part of ticcutils

  ticcutils is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  ticcutils is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>.

  For questions and suggestions, see:
      https://github.com/LanguageMachines/ticcutils/issues
  or send mail to:
      lamasoftware (at ) science.ru.nl
*/

#ifndef TICC_FILE_UTILS_H
#define TICC_FILE_UTILS_H

#include <vector>
#include <string>
#include <fstream>

namespace TiCC {
  std::vector<std::string> searchFilesExt( const std::string&,
					   const std::string&,
					   bool = true );
  inline std::vector<std::string> searchFiles( const std::string& name,
					bool recurse = true ){
    return searchFilesExt( name, "", recurse );
  }
  std::vector<std::string> searchFilesMatch( const std::string&,
					     const std::string&,
					     bool = true );
  bool isDir( const std::string& );
  bool isWritableDir( const std::string& );
  bool isFile( const std::string& );

  bool createPath( const std::string& );

  void erase( const std::string& );

  /// a class to maintain a temporary named stream
  class tmp_stream {
  public:
    explicit tmp_stream( const std::string& pf, bool keep = false ):
      tmp_stream( pf, "/tmp/", keep ) {};
    tmp_stream( const std::string&, const std::string&, bool = false );
    ~tmp_stream();
    void close()
    /// close the associated file
    { _os->close(); };
    std::ofstream& os()
      /// return a reference to the associated file
      { return *_os; };
    const std::string& tmp_name()
    /// return the generated name of the associated file
    { return _temp_name; };
  private:
    std::string _temp_name;
    std::ofstream *_os;
    bool _keep;
    tmp_stream( const tmp_stream& );
    tmp_stream operator=( const tmp_stream& );
  };

}

#endif // TICC_FILE_UTILS_H
