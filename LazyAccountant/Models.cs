using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;

namespace LazyAccountant
{
    public class Employee
    {
        public string  m_id = string.Empty;
        public string  m_name = string.Empty;
        public string  m_email = string.Empty;

        public decimal m_internalSalary = 0.0M;
        public decimal m_externalSalary = 0.0M;

        public decimal m_socialInsuranceCut = 0.0M;//社保扣款
        public decimal m_houseFundCut = 0.0M;//住房公积金扣款

        public decimal GetTotalSalary()
        {
            return m_internalSalary + m_externalSalary;
        }

        public override string ToString()
        {
            StringBuilder sb = new StringBuilder();
            sb.AppendFormat("工号:   {0}\n", m_id);
            sb.AppendFormat("姓名:   {0}\n", m_name);
            sb.AppendFormat("邮箱:   {0}\n", m_email);
            sb.AppendFormat("账内工资:   {0:0.00}\n", m_internalSalary);
            sb.AppendFormat("账外工资:   {0:0.00}\n", m_externalSalary);
            sb.AppendFormat("社保扣款:   {0:0.00}\n", m_socialInsuranceCut);
            sb.AppendFormat("公积金扣款: {0:0.00}\n", m_houseFundCut);
            return sb.ToString();
        }
    }

    //计算工资需要的关键数据
    public class CalcArg
    {
        public string m_employeeId = string.Empty;//员工

        public string m_month = string.Empty;//e.g. 201005

        public decimal m_allowance = 0.0M;//奖金, 补贴等额外收入

        public int m_late = 0;//迟到次数

        public float m_absent = 0.0F;//缺勤天数

        public decimal m_previousTaxCut = 0.0M;//上个月个税

        public decimal m_otherCut = 0.0M;//其他扣除金额

        public override string ToString()
        {
            return string.Format("{0},{1:0.00},{2},{3:0.00},{4:0.00},{5:0.00}",
                m_employeeId,//0
                m_allowance, //2
                m_late, //3, int
                m_absent, //4, float
                m_previousTaxCut, //5
                m_otherCut);//6
        }

        public void FromString(string str)
        {
            string[] infos = str.Split(',');
            int index = 0;
            m_employeeId = infos[index];
            decimal.TryParse(infos[++index], out m_allowance);
            int.TryParse(infos[++index], out m_late);
            float.TryParse(infos[++index], out m_absent);
            decimal.TryParse(infos[++index], out m_previousTaxCut);
            decimal.TryParse(infos[++index], out m_otherCut);
        }
    }

    //工资数据
    public class Salary
    {
        //这里需要保存一份员工信息, 而不仅仅是id, 因为员工信息会发生变化, 这里是保存计薪当时的员工数据
        public Employee m_employee = new Employee();

        public CalcArg m_args = new CalcArg();

        public decimal m_lateCut;//迟到扣款

        public decimal m_absentCut;//缺勤扣款

        public decimal m_incomeToTax;//应纳税收入

        public decimal m_taxToCut;//个税, 根据本月账内收入算出, 下个月再扣除

        public decimal m_internalIncome;

        public decimal m_externalIncome;

        public decimal m_totalIncome;

        //智诺计薪方法:
        //扣掉社保和公积金
        //按公司规定增加和扣除, 例如奖金, 迟到, 缺勤, 以及其他, 如损坏公物的赔偿    
        //计算个税(账内收入), 但这个月的个税留到下个月再扣除
        //扣掉上个月个税, 注意这里是先计算个税, 然后再扣除上个月的个税, 有点不合理
        //得到实发金额
        public void Calc(int workdayCount)
        {
            //计算迟到扣款
            m_lateCut = m_args.m_late * DataCenter.Instance.LateCutUnit;

            //缺勤扣款
            m_absentCut = m_employee.GetTotalSalary() * (decimal)m_args.m_absent / workdayCount;

            //计算应纳税收入
            m_incomeToTax = m_employee.m_internalSalary - m_employee.m_socialInsuranceCut - m_employee.m_houseFundCut + m_args.m_allowance - m_lateCut - m_absentCut - m_args.m_otherCut;

            m_taxToCut = IndividualIncomeTax.GetTax(DataCenter.Instance.IndividualIncomeTaxStart, m_incomeToTax);//计算个税, 但这个月不扣

            //总计数据
            m_internalIncome = m_incomeToTax - m_args.m_previousTaxCut;//扣上个月的税

            m_externalIncome = m_employee.m_externalSalary;

            m_totalIncome = m_internalIncome + m_externalIncome;
        }

        public string ToExcelLine()
        {
            StringBuilder b = new StringBuilder();
            b.AppendFormat("{0}\t{1}\t{2:0.00}\t{3:0.00}\t{4:0.00}\t{5:0.00}\t{6:0.00}\t{7:0.00}\t{8:0.00}\t{9:0.00}\t{10:0.00}\t{11:0.00}",
                m_employee.m_id, m_employee.m_name,
                    m_employee.m_internalSalary, m_employee.m_externalSalary,
                    m_lateCut, m_absentCut, m_args.m_allowance,
                    m_employee.GetTotalSalary() - m_lateCut - m_absentCut,
                    m_employee.m_socialInsuranceCut, m_employee.m_houseFundCut, m_args.m_previousTaxCut,
                    m_totalIncome);
            return b.ToString();
        }

        public override string ToString()
        {
            if (m_totalIncome <= 0.0M)
            {
                return string.Empty;
            }

            StringBuilder sb = new StringBuilder();

            sb.AppendLine(String.Format("员工姓名:       {0}", m_employee.m_name));

            if (m_args.m_allowance > 0.0M)
            {
                sb.AppendLine(String.Format("奖金补贴:       {0:0.00}", m_args.m_allowance));
            }

            if (m_lateCut > 0.0M)
            {
                sb.AppendLine(String.Format("迟到扣款:       {0:0.00}", m_lateCut));
            }

            if (m_absentCut > 0.0M)
            {
                sb.AppendLine(String.Format("缺勤扣款:       {0:0.00}", m_absentCut));
            }

            if (m_employee.m_socialInsuranceCut > 0.0M)
            {
                sb.AppendLine(String.Format("社保扣款:       {0:0.00}", m_employee.m_socialInsuranceCut));
            }

            if (m_employee.m_houseFundCut > 0.0M)
            {
                sb.AppendLine(String.Format("公积金扣款:     {0:0.00}", m_employee.m_houseFundCut));
            }

            if (m_args.m_otherCut > 0.0M)
            {
                sb.AppendLine(String.Format("其他扣款:       {0:0.00}", m_args.m_otherCut));
            }

            if (m_incomeToTax > 0.0M)
            {
                sb.AppendLine(String.Format("应纳税收入:     {0:0.00}", m_incomeToTax));
            }

            //if (m_taxToCut > 0)
            //{
            //    sb.AppendLine(String.Format("本月应缴个税:   {0:0.00}(下个月再缴)", m_taxToCut));
            //}

            if (m_args.m_previousTaxCut > 0.0M)
            {
                sb.AppendLine(String.Format("上月个税扣款:   {0:0.00}(上个月未缴, 本月扣除)", m_args.m_previousTaxCut));
            }

            if (m_externalIncome > 0.0M)
            {
                sb.AppendLine(String.Format("账内收入:       {0:0.00}", m_internalIncome));

                sb.AppendLine(String.Format("账外收入:       {0:0.00}", m_externalIncome));
            }

            sb.AppendLine(String.Format("全部收入:       {0:0.00}", m_totalIncome));

            return sb.ToString();
        }

        //获取一个时间范围内有多少个工作日, 不包括to那一天, 例如, 参数为2010.04.01 - 2010.05.01, 算出的是4月份的应工作天数
        public static int GetWorkdayCount(DateTime from, DateTime to)
        {
            DateTime date = from;
            int workdayCount = 0;
            while (date < to)
            {
                if (date.DayOfWeek != DayOfWeek.Saturday && date.DayOfWeek != DayOfWeek.Sunday)
                {
                    ++workdayCount;
                }
                date = date.AddDays(1);
            }
            return workdayCount;
        }

        //获取某年的某个月有多少个工作日
        public static int GetWorkdayCount(int year, int month)
        {
            DateTime from = new DateTime(year, month, 1);
            DateTime to = from.AddMonths(1);
            return GetWorkdayCount(from, to);
        }

        public static int GetWorkdayCount(DateTime month)
        {
            DateTime from = new DateTime(month.Year, month.Month, 1);
            DateTime to = from.AddMonths(1);
            return GetWorkdayCount(from, to);
        }
    }
}
