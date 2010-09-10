using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.IO;

namespace LazyAccountant
{
    //数据库
    public class DataCenter
    {
        private static DataCenter m_instance = new DataCenter();

        public static DataCenter Instance
        {
            get { return DataCenter.m_instance; }
        }

        private Dictionary<string, Employee> m_employees = new Dictionary<string, Employee>();


        public Dictionary<string, Employee> Employees
        {
            get { return m_employees; }
        }


        private decimal m_lateCutUnit = 10.0M;

        public decimal LateCutUnit
        {
            get { return m_lateCutUnit; }
        }


        private decimal m_individualIncomeTaxStart = 2000.0M;

        public decimal IndividualIncomeTaxStart
        {
            get { return m_individualIncomeTaxStart; }
        }

        private string m_mailServer = String.Empty;

        public string MailServer
        {
            get { return m_mailServer; }
        }

        private string m_mailUsername = String.Empty;

        public string MailUsername
        {
            get { return m_mailUsername; }
        }


        private string m_mailPassword = String.Empty;

        public string MailPassword
        {
            get { return m_mailPassword; }
        }

        private DataCenter()
        {
            try
            {
                LoadData();
            }
            catch (System.Exception)
            {
            	
            }
        }

        public void LoadData()
        {
            XmlDocument doc = new XmlDocument();
            String file = "ZenoIntel.xml";
            if (!File.Exists(file))
            {
                return;
            }

            doc.Load(file);

            XmlElement root = doc.DocumentElement;//ZenoIntel

            foreach (XmlNode node in root.ChildNodes)
            {
                //只处理Element
                if (node.NodeType != XmlNodeType.Element)
                {
                    continue;
                }

                if (node.Name == "Employees")
                {
                    foreach (XmlNode employeeNode in node.ChildNodes)
                    {
                        if (employeeNode.NodeType == XmlNodeType.Element && employeeNode.Name == "Employee")
                        {
                            Employee e = new Employee();
                            e.m_id = employeeNode.Attributes["id"].Value;
                            e.m_name = employeeNode.Attributes["name"].Value;
                            e.m_email = employeeNode.Attributes["email"].Value;
                            e.m_internalSalary = decimal.Parse(employeeNode.Attributes["internalSalary"].Value);
                            e.m_externalSalary = decimal.Parse(employeeNode.Attributes["externalSalary"].Value);
                            e.m_socialInsuranceCut = decimal.Parse(employeeNode.Attributes["socialInsuranceCut"].Value);
                            e.m_houseFundCut = decimal.Parse(employeeNode.Attributes["houseFundCut"].Value);
                            m_employees.Add(e.m_id, e);
                        }
                    }
                }
                else if (node.Name == "LateCutUnit")
                {
                    m_lateCutUnit = decimal.Parse(node.InnerText);
                }
                else if (node.Name == "IndividualIncomeTaxStart")
                {
                    m_individualIncomeTaxStart = decimal.Parse(node.InnerText);
                }
                else if (node.Name == "Email")
                {
                    m_mailServer = node.Attributes["server"].Value;
                    m_mailUsername = node.Attributes["username"].Value;
                    m_mailPassword = node.Attributes["password"].Value;
                }
            }
        }

        public void SaveSalaries(string file, ICollection<Salary> salaries)
        {
            if (String.IsNullOrEmpty(file))
            {
                return;
            }

            if (File.Exists(file))
            {
                File.Delete(file);
            }

            using(StreamWriter sw = File.CreateText(file))
            {
                foreach (Salary s in salaries)
                {
                    String line = String.Format("{0},{1:0.00},{2:0.00},{3:0.00},{4:0.00},{5:0.00},{6},{7:0.00},{8:0.00},{9:0.00},{10:0.00},{11:0.00},{12:0.00},{13:0.00},{14:0.00},{15:0.00},{16:0.00}", 
                        s.m_employee.m_id,//0, string
                        s.m_employee.m_internalSalary,
                        s.m_employee.m_externalSalary,
                        s.m_employee.m_socialInsuranceCut,
                        s.m_employee.m_houseFundCut,

                        s.m_args.m_allowance,//5
                        s.m_args.m_late,//6, int
                        s.m_args.m_absent,//7, float
                        s.m_args.m_previousTaxCut,
                        s.m_args.m_otherCut,

                        s.m_lateCut, //10
                        s.m_absentCut,
                        s.m_incomeToTax,
                        s.m_taxToCut,//13

                        s.m_internalIncome,//14
                        s.m_externalIncome,
                        s.m_totalIncome);//16

                    sw.WriteLine(line);
                }
            }
        }

        //目前只需要加载个税数据
        public Dictionary<string, Salary> LoadSalaries(String file)
        {
            Dictionary<string, Salary> salaries = new Dictionary<string, Salary>();

            if (String.IsNullOrEmpty(file))
            {
                return salaries;
            }
            else if (!File.Exists(file))
            {
                return salaries;
            }

            using (StreamReader reader = File.OpenText(file))
            {
                string line = string.Empty;
                while (!string.IsNullOrEmpty((line = reader.ReadLine())))
                {
                    string[] data = line.Split(',');
                    if (data != null && data.Length >=2)
                    {
                        Salary s = new Salary();

                        s.m_employee.m_id = data[0];
                        s.m_args.m_employeeId = data[0];
                        
                        decimal.TryParse(data[13], out s.m_taxToCut);//tax to cut

                        salaries.Add(s.m_employee.m_id, s);
                    }
                }
            }

            return salaries;
        }

        public void SaveCalcArgs(string file, ICollection<CalcArg> calcArgs)
        {
            if (String.IsNullOrEmpty(file))
            {
                return;
            }

            if (File.Exists(file))
            {
                File.Delete(file);
            }

            using(StreamWriter sw = File.CreateText(file))
            {
                foreach (CalcArg arg in calcArgs)
                {
                    sw.WriteLine(arg.ToString());
                }
            }
        }

        public Dictionary<string, CalcArg> LoadCalcArgs(string file)
        {
            Dictionary<string, CalcArg> calcArgs = new Dictionary<string, CalcArg>();

            if (String.IsNullOrEmpty(file))
            {
                return calcArgs;
            }
            else if (!File.Exists(file))
            {
                return calcArgs;
            }

            using (StreamReader reader = File.OpenText(file))
            {
                string line = string.Empty;
                while (!string.IsNullOrEmpty((line = reader.ReadLine())))
                {
                    CalcArg arg = new CalcArg();
                    arg.FromString(line);
                    if (!string.IsNullOrEmpty(arg.m_employeeId))
                    {
                        calcArgs.Add(arg.m_employeeId, arg);
                    }
                }
            }

            return calcArgs;
        }
    }
}
