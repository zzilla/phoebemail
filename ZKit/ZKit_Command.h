#ifndef _ZKit_Command_h_
#define _ZKit_Command_h_

#include "ZKit_TLV.h"
#include "ZKit_IDGenerator.h"

BEGIN_ZKIT
//////////////////////////////////////////////////////////////////////////
class Command : public ISerializable
{
	friend std::ostream& operator << (std::ostream& os, const Command& cmd);
public:
	Command(void);

	~Command(void);

	Command& operator = (const Command& cmd);

	Command(const Command& cmd);

	void Init(uint16 cmdType, uint16 cmdId);

	void InitAck(Command& ack) const;

	uint16 GetCmdType() const
	{
		return m_cmdType;
	}

	uint16 GetCmdId() const
	{
		return m_cmdId;
	}

	uint32 GetSessionId() const
	{
		return m_sessionId;
	}

	void AddArg(const TLV& tlv);

	bool HasArgs() const
	{
		return !m_args.empty();
	}

	bool ArgExists(int tlvType) const
	{
		return m_args.find(tlvType) != m_args.end();
	}

	bool IsAck() const;

	uint32 GetTotalLength() const;

	const TLV& GetArg(uint16 tlvType) const
	{
		return m_args[tlvType];
	}

	bool Equals(const Command& rhs) const
	{
		return m_cmdType == rhs.m_cmdType && m_cmdId == rhs.m_cmdId && m_sessionId == rhs.m_sessionId;
	}

	bool operator == (const Command& rhs) const
	{
		return this->Equals(rhs);
	}

	uint32 GetArgsLength() const;

	std::string ToString() const;

	static bool Check( const std::vector<char>& bytes );

	virtual bool Serialize(std::vector<char>& bytes) const;

	virtual bool Deserialize(const std::vector<char>& bytes);
private:
	uint16 m_cmdType;
	uint16 m_cmdId;
	uint32 m_sessionId;
	mutable std::map<uint16, TLV> m_args;
	static IDGenerator m_idGen;
};

//注意<< 和 >> 不应该是成员函数, 但它们是类接口
std::ostream& operator << (std::ostream& os, const Command& cmd);

END_ZKIT
#endif // _ZKit_Command_h_
