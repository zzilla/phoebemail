#include "ZKit_BitConverter.h"
#include "ZKit_Command.h"
BEGIN_ZKIT

IDGenerator Command::m_idGen;

bool Command::Serialize( std::vector<char>& bytes ) const
{
	bytes.clear();
	bytes.push_back((char)CMD_FLAG_BYTE0);
	bytes.push_back((char)CMD_FLAG_BYTE1);
	BitConverter::AppendValue(bytes, m_cmdType);
	BitConverter::AppendValue(bytes, m_cmdId);
	BitConverter::AppendValue(bytes, GetArgsLength());
	BitConverter::AppendValue(bytes, m_sessionId);

	bytes.insert(bytes.end(), CMD_LEN_RESERVED, 0);//保留字段全部填充0

	std::vector<char> tlvBytes;	
	for (std::map<uint16, TLV>::const_iterator i = m_args.begin(); i != m_args.end(); ++i)
	{
		tlvBytes.clear();
		(*i).second.Serialize(tlvBytes);
		bytes.insert(bytes.end(), tlvBytes.begin(), tlvBytes.end());
	}

	return true;
}

bool Command::Deserialize( const std::vector<char>& bytes )
{
	if (!Check(bytes))
	{
		return false;
	}

	BitConverter::GetValue(&bytes[0], CMD_IDX_TYPE, m_cmdType);
	BitConverter::GetValue(&bytes[0], CMD_IDX_ID, m_cmdId);
	uint32 argsLength = 0;
	BitConverter::GetValue(&bytes[0], CMD_IDX_ARGSLEN, argsLength);
	BitConverter::GetValue(&bytes[0], CMD_IDX_SESSIONID, m_sessionId);

	if (argsLength > 0)
	{
		uint32 i = 0;
		while (i < argsLength)
		{
			std::vector<char> tlvBytes;
			tlvBytes.insert(tlvBytes.end(), bytes.begin() + CMD_LEN_HEADER + i, bytes.end());
			TLV tlv;
			if (tlv.Deserialize(tlvBytes))
			{
				i += tlv.GetTotalLength();
				m_args[tlv.GetType()] = tlv;
			}
			else
			{
				return false;
			}
		}
	}

	return true;
}

std::string Command::ToString() const
{
	std::ostringstream oss;
	oss << *this;
	return oss.str();
}

bool Command::Check( const std::vector<char>& bytes )
{
	if (bytes.size() < CMD_LEN_HEADER)
	{
		return false;
	}

	if (bytes[0] != (char)CMD_FLAG_BYTE0 || bytes[1] != (char)CMD_FLAG_BYTE1)
	{
		return false;
	}

	uint32 argsLength = 0;
	BitConverter::GetValue(&bytes[0], CMD_IDX_ARGSLEN, argsLength);

	if (bytes.size() < CMD_LEN_HEADER + argsLength)
	{
		return false;
	}

	return true;
}

uint32 Command::GetArgsLength() const
{
	uint32 argsLength = 0;
	for (std::map<uint16, TLV>::const_iterator i = m_args.begin(); i != m_args.end(); ++i)
	{
		argsLength += (*i).second.GetTotalLength();
	}
	return argsLength;
}

Command::Command( void ) : m_cmdType(0), m_cmdId(0), m_sessionId(0)
{

}

Command& Command::operator = (const Command& cmd)
{
	if (&cmd != this)
	{
		m_cmdType = cmd.m_cmdType;
		m_cmdId = cmd.m_cmdId;
		m_sessionId = cmd.m_sessionId;
		m_args.insert(cmd.m_args.begin(), cmd.m_args.end());
	}

	return *this;
}

Command::Command( const Command& cmd )
{
	*this = cmd;
}

Command::~Command( void )
{
	m_args.clear();
}

void Command::Init( uint16 cmdType, uint16 cmdId)
{
	m_cmdType = cmdType;
	m_cmdId = cmdId;
	m_sessionId = m_idGen.getId();//每次调用Init, 会自动生成新的sessionId
}

void Command::InitAck( Command& ack ) const
{
	ack.m_cmdType = m_cmdType;
	ack.m_cmdId = BUILD_ACK(m_cmdId);
	ack.m_sessionId = m_sessionId;
}

bool Command::IsAck() const
{
	return IS_ACK(m_cmdId);
}

uint32 Command::GetTotalLength() const
{
	return GetArgsLength() + CMD_LEN_HEADER;
}

void Command::AddArg( const TLV& tlv )
{
	m_args[tlv.GetType()] = tlv;
}

std::ostream& operator<<( std::ostream& oss, const Command& cmd )
{
	oss << hex << "\nCmdType: 0x" << cmd.GetCmdType() << "  CmdId: 0x" << cmd.GetCmdId() << dec << "  SessionId: " << cmd.GetSessionId() << "  ArgsLength: " << cmd.GetArgsLength();
	if (cmd.GetArgsLength() > 0)
	{
		oss << "\nArgs:";
		for (std::map<uint16, TLV>::const_iterator i = cmd.m_args.begin(); i != cmd.m_args.end(); ++i)
		{
			oss << (*i).second;
		}
	}
	return oss;
}
END_ZKIT
