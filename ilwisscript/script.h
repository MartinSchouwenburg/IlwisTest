/*IlwisObjects is a framework for analysis, processing and visualization of remote sensing and gis data
Copyright (C) 2018  52n North

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.*/

#ifndef EXECUTESCRIPT_H
#define EXECUTESCRIPT_H

namespace Ilwis {

class Script : public OperationImplementation
{
public:
    Script();
    Script(quint64 metaid, const Ilwis::OperationExpression &expr);

    bool execute(ExecutionContext *ctx, SymbolTable &extsym);
    OperationImplementation::State prepare(Ilwis::ExecutionContext *ctx, const Ilwis::SymbolTable &);
    static Ilwis::OperationImplementation *create(quint64 metaid,const Ilwis::OperationExpression& expr);

    static quint64 createMetadata();

private:
    std::unique_ptr<char []> _buffer;
    quint32 _bufferSize;

    bool detectKey(const std::string &line, const std::string &key);

};
}

#endif // EXECUTESCRIPT_H
