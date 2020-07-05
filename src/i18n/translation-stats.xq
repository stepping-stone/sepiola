(:
  sepiola - Open Source Online Backup Client
  Copyright (c) 2007-2020 stepping stone AG
 
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
:)

xquery version "1.0";

declare function local:translation-file-stats ($file as xs:string) as node()
{
    let $ts := doc($file)/TS
    return
        <translation file="{$file}" language="{$ts/@language}">
            <total>{fn:count($ts/context/message/translation)}</total>
            <unfinished>{fn:count($ts/context/message/translation[attribute::type = "unfinished"])}</unfinished>
        </translation>
};

(: space-delimited list of translation files :)
let $translations := ( 'app_de.ts' )

return
    <translations>
    {
        for $t in $translations
        return local:translation-file-stats($t)
    }
    </translations>
