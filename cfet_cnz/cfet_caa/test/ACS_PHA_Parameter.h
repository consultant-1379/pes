#ifndef ACS_PHA_PARAMETER_
#define ACS_PHA_PARAMETER_

typedef int ACS_PHA_PARAM_RC;

int ACS_PHA_PARAM_RC_OK = 0;

template <typename T>
class ACS_PHA_Parameter
{
public:
	ACS_PHA_Parameter(const char name[]):
	m_t()
	{
	}

	ACS_PHA_PARAM_RC get(ACS_PHA_Tables& table) 
	{
		return ACS_PHA_PARAM_RC_OK;
	}

	const T& data() const 
	{
		//return m_t;
                  return 4;
	}

private:
	T m_t;
};

#endif
